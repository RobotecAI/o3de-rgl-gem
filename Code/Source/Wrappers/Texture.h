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

namespace RGL::Wrappers
{
    class Entity;

    class Texture
    {
    public:
        static const Texture& GetGlobalDebugTexture();
        static Texture CreatePlaceholder();
        static Texture CreateFromMaterialAsset(const AZ::Data::Asset<AZ::RPI::MaterialAsset>& materialAsset);

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
        // 2x2 black and white checkerboard bitmap
        static std::vector<uint8_t> DebugBitmap;

        rgl_texture_t m_nativePtr{ nullptr };

        friend class Entity;
    };
} // namespace RGL::Wrappers
