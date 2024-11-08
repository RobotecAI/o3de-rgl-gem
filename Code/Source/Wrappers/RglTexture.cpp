/* Copyright 2024, Robotec.ai sp. z o.o.
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
#include <Utilities/BlockCompression.h>
#include <Utilities/RGLUtils.h>
#include <Wrappers/RglTexture.h>

namespace RGL::Wrappers
{
    RglTexture RglTexture::CreateFromMaterialAsset(const AZ::Data::Asset<AZ::RPI::MaterialAsset>& materialAsset)
    {
        static const AZStd::string TraceWindowName = ConstructTraceWindowName(__func__);
        static const AZ::Name albedoTexName = AZ::Name::FromStringLiteral("baseColor.textureMap", AZ::Interface<AZ::NameDictionary>::Get());
        static const AZ::Name albedoColorName = AZ::Name::FromStringLiteral("baseColor.color", AZ::Interface<AZ::NameDictionary>::Get());

        RglTexture imageRglTexture = CreateInvalid();

        const AZ::Data::AssetId& id = materialAsset.GetId();
        if (!materialAsset.IsReady())
        {
            AZ_Warning(
                TraceWindowName.c_str(), false, "The material asset with ID: %s was not ready.", id.ToString<AZStd::string>().c_str());
            return imageRglTexture;
        }

        const auto* propLayout = materialAsset->GetMaterialPropertiesLayout();

        if (!propLayout)
        {
            AZ_Warning(
                TraceWindowName.c_str(),
                false,
                "Unable to access material properties layout of material asset with ID: %s.",
                id.ToString<AZStd::string>().c_str());
            return imageRglTexture;
        }

        auto& propertyValues = materialAsset->GetPropertyValues();

        if (const auto albedoTexPropIdx = propLayout->FindPropertyIndex(albedoTexName); !albedoTexPropIdx.IsNull())
        {
            const auto albedoTexImagePropVal = propertyValues.at(albedoTexPropIdx.GetIndex());
            const auto& imageAsset = albedoTexImagePropVal.GetValue<AZ::Data::Asset<AZ::RPI::ImageAsset>>();
            imageRglTexture = CreateFromImageAsset(imageAsset.GetId());
        }

        if (imageRglTexture.IsValid())
        {
            return imageRglTexture;
        }

        if (const auto albedoColorPropIdx = propLayout->FindPropertyIndex(albedoColorName); !albedoColorPropIdx.IsNull())
        {
            imageRglTexture =
                AZStd::move(CreateFromFactor(CreateGrayFromColor(propertyValues.at(albedoColorPropIdx.GetIndex()).GetValue<AZ::Color>())));
        }
        else
        {
            AZ_Error(
                TraceWindowName.c_str(),
                false,
                "Unable to find specular color and texture properties of material asset with ID: %s.",
                id.ToString<AZStd::string>().c_str());
        }

        return imageRglTexture;
    }

    RglTexture RglTexture::CreateFromFactor(float factor)
    {
        auto factor8 = aznumeric_cast<AZ::u8>(factor * 255.0f);
        return RglTexture{ &factor8, 1, 1 };
    }

    float RglTexture::CreateGrayFromColor(const AZ::Color& color)
    {
        return (RedGrayMultiplier * color.GetR() + GreenGrayMultiplier * color.GetG() + BlueGrayMultiplier * color.GetB()) * color.GetA();
    }

    uint8_t RglTexture::CreateGray8FromColor(const AZ::Color& color)
    {
        return static_cast<uint8_t>(CreateGrayFromColor(color) * 255.0f);
    }

    RglTexture RglTexture::CreateFromImageAsset(const AZ::Data::AssetId& imageAssetId)
    {
        // Made a static member to minimize reallocations, since multiple assets will be processed.
        static AZStd::vector<uint8_t> tempRglTextureData;

        RglTexture imageRglTexture = CreateInvalid();

        if (!imageAssetId.IsValid())
        {
            return imageRglTexture;
        }

        AZ::Data::Asset<AZ::RPI::StreamingImageAsset> imageAsset =
            AZ::Data::AssetManager::Instance().GetAsset<AZ::RPI::StreamingImageAsset>(
                imageAssetId,
                AZ::Data::AssetLoadBehavior::PreLoad,
                AZ::Data::AssetLoadParameters(nullptr, AZ::Data::AssetDependencyLoadRules::LoadAll));
        imageAsset.QueueLoad();
        imageAsset.BlockUntilLoadComplete();

        const AZ::RHI::ImageDescriptor imageDescriptor = imageAsset->GetImageDescriptor();

        using Format = AZ::RHI::Format;
        const auto format = imageDescriptor.m_format;
        const auto& size = imageDescriptor.m_size;
        static AZStd::set<AZ::RHI::Format> SupportedFormats{
            Format::BC1_UNORM, Format::BC1_UNORM_SRGB, Format::BC3_UNORM, Format::BC3_UNORM_SRGB, Format::BC4_UNORM,
        };
        if (SupportedFormats.contains(format))
        {
            tempRglTextureData.resize(size.m_width * size.m_height);

            // Only highest detail mip.
            AZStd::span<const uint8_t> imageData = imageAsset->GetSubImageData(0, 0);
            size_t srcIdx = 0, sliceIdx = 0;
            for (size_t y = 0; y < size.m_height; y += 4)
            {
                for (size_t x = 0; x < size.m_width; x += 4)
                {
                    if (format == Format::BC4_UNORM)
                    {
                        LoadBlockToGrays(
                            reinterpret_cast<const AZ::RPI::BC4Block*>(imageData.data() + srcIdx), tempRglTextureData, x, y, size.m_width);
                    }
                    else if (format == Format::BC1_UNORM || format == Format::BC1_UNORM_SRGB)
                    {
                        LoadBlockToGrays(
                            reinterpret_cast<const AZ::RPI::BC1Block*>(imageData.data() + srcIdx), tempRglTextureData, x, y, size.m_width);
                    }
                    else if (format == Format::BC3_UNORM || format == Format::BC3_UNORM_SRGB)
                    {
                        LoadBlockToGrays(
                            reinterpret_cast<const Utils::BC3Block*>(imageData.data() + srcIdx), tempRglTextureData, x, y, size.m_width);
                    }

                    srcIdx += GetFormatSize(format);
                    if (srcIdx == imageData.size() && ++sliceIdx < imageDescriptor.m_arraySize)
                    {
                        imageData = imageAsset->GetSubImageData(0, sliceIdx);
                        srcIdx = 0;
                    }
                }
            }

            imageRglTexture = RglTexture{ tempRglTextureData.data(), size.m_width, size.m_height };
        }
        else
        {
            AZ_Warning(
                ConstructTraceWindowName(__func__).c_str(),
                false,
                "Image \"%s\" is of unsupported type: %s. Only BC1 and BC4 formats are currently supported. Skipping...",
                imageAsset.ToString<AZStd::string>().c_str(),
                ToString(imageDescriptor.m_format));
        }

        return imageRglTexture;
    }

    RglTexture::RglTexture(const uint8_t* texels, size_t width, size_t height)
    {
        bool success = false;
        Utils::ErrorCheck(
            rgl_texture_create(&m_nativePtr, texels, aznumeric_cast<int32_t>(width), aznumeric_cast<int32_t>(height)),
            __FILE__,
            __LINE__,
            &success);

        if (!success && m_nativePtr)
        {
            RGL_CHECK(rgl_texture_destroy(m_nativePtr));
            m_nativePtr = nullptr;
        }
    }

    RglTexture::RglTexture(RglTexture&& other)
    {
        if (IsValid())
        {
            RGL_CHECK(rgl_texture_destroy(m_nativePtr));
        }

        m_nativePtr = other.m_nativePtr;
        other.m_nativePtr = nullptr;
    }

    RglTexture::~RglTexture()
    {
        if (IsValid())
        {
            RGL_CHECK(rgl_texture_destroy(m_nativePtr));
        }
    }

    RglTexture& RglTexture::operator=(RglTexture&& other)
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
