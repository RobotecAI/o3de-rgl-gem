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
#pragma once

#include <Atom/RPI.Reflect/Image/ImageAsset.h>
#include <Atom/RPI.Reflect/Material/MaterialAsset.h>
#include <AzCore/Asset/AssetCommon.h>
#include <rgl/api/core.h>

namespace AZ::RPI
{
    struct BC1Block;
    struct BC4Block;
} // namespace AZ::RPI

namespace RGL::Wrappers
{
    class Entity;

    class Texture
    {
        friend class Entity;

    public:
        static Texture CreateInvalid()
        {
            return {};
        }

        static const Texture& GetGlobalDebugTexture();
        static Texture CreatePlaceholder();
        //! This function is not thread safe.
        static Texture CreateFromMaterialAsset(const AZ::Data::Asset<AZ::RPI::MaterialAsset>& materialAsset);
        static Texture CreateFromFactor(float factor);
        //! This function is not thread safe. Currently, only images in the BC1 format are supported.
        static Texture CreateFromImageAsset(const AZ::Data::AssetId& imageAssetId);

        Texture(const uint8_t* texels, size_t width, size_t height);
        Texture(const Texture& other) = delete;
        Texture(Texture&& other);
        ~Texture();

        [[nodiscard]] bool IsValid() const
        {
            return m_nativePtr;
        }

        Texture& operator=(const Texture& other) = delete;
        Texture& operator=(Texture&& other);

    private:
        //! Creates an invalid texture.
        //! To avoid creating an invalid texture by accident, it is private.
        //! See CreateInvalid.
        Texture()
        {
        }

        static AZStd::string ConstructTraceWindow(const char* functionName)
        {
            return (AZStd::string("RGL::Texture::") + functionName);
        }

        static uint8_t Gray8FromColor(const AZ::Color& color);

        template<typename BlockT>
        static void LoadBlockToGrays(
            const BlockT* block, AZStd::vector<uint8_t>& grayValues, size_t blockX, size_t blockY, size_t width)
        {
            size_t i = 0;
            for (size_t y = blockY; y < blockY + 4; ++y)
            {
                for (size_t x = blockX; x < blockX + 4; ++x)
                {
                    grayValues[x + y * width] = Gray8FromColor(block->GetBlockColor(i));
                    ++ i;
                }
            }
        }

        static constexpr float RedGrayMultiplier = 0.299f;
        static constexpr float GreenGrayMultiplier = 0.587f;
        static constexpr float BlueGrayMultiplier = 0.114f;

        rgl_texture_t m_nativePtr{ nullptr };
    };
} // namespace RGL::Wrappers
