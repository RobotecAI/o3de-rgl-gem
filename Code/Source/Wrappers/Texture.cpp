/* Copyright 2020-2021, Robotec.ai sp. z o.o.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <Atom/RPI.Public/BlockCompression.h>
#include <AzCore/Name/NameDictionary.h>
#include <Wrappers/Texture.h>

namespace RGL::Wrappers
{
    Texture Texture::CreateFromMaterialAsset(const AZ::Data::Asset<AZ::RPI::MaterialAsset>& materialAsset)
    {
        static const AZStd::string TraceWindow = ConstructTraceWindow(__func__);
        static const AZ::Name specTexName = AZ::Name::FromStringLiteral("specularF0.textureMap", AZ::Interface<AZ::NameDictionary>::Get());
        static const AZ::Name specFactorName = AZ::Name::FromStringLiteral("specularF0.factor", AZ::Interface<AZ::NameDictionary>::Get());

        Texture imageTexture = CreateInvalid();

        const AZ::Data::AssetId& id = materialAsset.GetId();
        if (!materialAsset.IsReady())
        {
            AZ_Warning(TraceWindow.c_str(), false, "The material asset with ID: %s was not ready.", id.ToString<AZStd::string>().c_str());
            return imageTexture;
        }

        const auto* propLayout = materialAsset->GetMaterialPropertiesLayout();

        if (!propLayout)
        {
            AZ_Warning(
                TraceWindow.c_str(),
                false,
                "Unable to access material properties layout of material asset with ID: %s.",
                id.ToString<AZStd::string>().c_str());
            return imageTexture;
        }

        auto& propertyValues = materialAsset->GetPropertyValues();

        if (const auto specTexPropIdx = propLayout->FindPropertyIndex(specTexName); !specTexPropIdx.IsNull())
        {
            const auto specTexImagePropVal = propertyValues.at(specTexPropIdx.GetIndex());
            const auto& imageAsset = specTexImagePropVal.GetValue<AZ::Data::Asset<AZ::RPI::ImageAsset>>();
            imageTexture = CreateFromImageAsset(imageAsset.GetId());
        }

        if (imageTexture.IsValid())
        {
            return imageTexture;
        }

        if (const auto specFactorPropIdx = propLayout->FindPropertyIndex(specFactorName); !specFactorPropIdx.IsNull())
        {
            imageTexture = AZStd::move(CreateFromFactor(propertyValues.at(specFactorPropIdx.GetIndex()).GetValue<float>()));
        }
        else
        {
            AZ_Error(
                TraceWindow.c_str(),
                false,
                "Unable to find specular color and texture properties of material asset with ID: %s.",
                id.ToString<AZStd::string>().c_str());
        }

        return imageTexture;
    }

    Texture Texture::CreateFromFactor(float factor)
    {
        auto factor8 = aznumeric_cast<AZ::u8>(factor * 255.0f);
        return Texture{ &factor8, 1, 1 };
    }

    uint8_t Texture::Gray8FromColor(const AZ::Color& color)
    {
        return static_cast<uint8_t>(
            (GreenGrayMultiplier * color.GetR() + GreenGrayMultiplier * color.GetG() + BlueGrayMultiplier * color.GetB()) * 255.0f);
    }

    Texture Texture::CreateFromImageAsset(const AZ::Data::AssetId& imageAssetId)
    {
        // Made a static member to minimize reallocations, since multiple assets will be processed.
        static AZStd::vector<uint8_t> tempRglTextureData;

        Texture imageTexture = CreateInvalid();

        if (!imageAssetId.IsValid())
        {
            return imageTexture;
        }

        AZ::Data::Asset<AZ::RPI::StreamingImageAsset> imageAsset =
            AZ::Data::AssetManager::Instance().GetAsset<AZ::RPI::StreamingImageAsset>(
                imageAssetId,
                AZ::Data::AssetLoadBehavior::PreLoad,
                AZ::Data::AssetLoadParameters(nullptr, AZ::Data::AssetDependencyLoadRules::LoadAll));
        imageAsset.QueueLoad();
        imageAsset.BlockUntilLoadComplete();

        const AZ::RHI::ImageDescriptor imageDescriptor = imageAsset->GetImageDescriptorForMipLevel(0);

        using Format = AZ::RHI::Format;
        const auto format = imageDescriptor.m_format;
        if (format == Format::BC1_UNORM || format == Format::BC1_UNORM_SRGB || format == Format::BC4_UNORM)
        {
            const size_t width = imageDescriptor.m_size.m_width;
            const size_t height = imageDescriptor.m_size.m_height;

            tempRglTextureData.resize(width * height);

            // Only highest detail mip and first slice (2D texture).
            AZStd::span<const uint8_t> imageData = imageAsset->GetSubImageData(0, 0);
            size_t srcIdx = 0;
            for (size_t y = 0; y < width; y += 4)
            {
                for (size_t x = 0; x < width; x += 4)
                {
                    if (format == Format::BC4_UNORM)
                    {
                        LoadBlockToGrays(
                            reinterpret_cast<const AZ::RPI::BC4Block*>(imageData.data() + srcIdx), tempRglTextureData, x, y, width);
                    }
                    else
                    {
                        LoadBlockToGrays(
                            reinterpret_cast<const AZ::RPI::BC1Block*>(imageData.data() + srcIdx), tempRglTextureData, x, y, width);
                    }
                    srcIdx += 8;
                }
            }

            imageTexture = Texture{ tempRglTextureData.data(), width, height };
            tempRglTextureData.clear();
        }
        else
        {
            AZ_Warning(
                ConstructTraceWindow(__func__).c_str(),
                false,
                "Image is of unsupported type: %s. Only BC1 and BC4 formats are currently supported. Skipping...",
                ToString(imageDescriptor.m_format));
        }

        return imageTexture;
    }

    Texture::Texture(const uint8_t* texels, size_t width, size_t height)
    {
        bool success = false;
        Utils::ErrorCheck(
            rgl_texture_create(&m_nativePtr, texels, aznumeric_cast<int32_t>(width), aznumeric_cast<int32_t>(height)),
            __FILE__,
            __LINE__,
            &success);

        if (!success && !m_nativePtr)
        {
            RGL_CHECK(rgl_texture_destroy(m_nativePtr));
            m_nativePtr = nullptr;
        }
    }

    Texture::Texture(Texture&& other)
        : m_nativePtr(other.m_nativePtr)
    {
        other.m_nativePtr = nullptr;
    }

    Texture::~Texture()
    {
        if (IsValid())
        {
            RGL_CHECK(rgl_texture_destroy(m_nativePtr));
        }
    }

    Texture& Texture::operator=(Texture&& other)
    {
        if (this != &other)
        {
            if (IsValid())
            {
                RGL_CHECK(rgl_texture_destroy(m_nativePtr));
            }

            m_nativePtr = other.m_nativePtr;
            other.m_nativePtr = nullptr;
        }

        return *this;
    }
} // namespace RGL::Wrappers
