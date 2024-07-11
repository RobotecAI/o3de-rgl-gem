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
#include <AzCore/Name/NameDictionary.h>
#include <Wrappers/Texture.h>

namespace RGL::Wrappers
{
    std::vector<uint8_t> Texture::DebugBitmap = { 0, 0xff, 0xff, 0 };

    const Texture& Texture::GetGlobalDebugTexture()
    {
        static Texture debugTexture = CreatePlaceholder();
        return debugTexture;
    }

    Texture Texture::CreatePlaceholder()
    {
        return { DebugBitmap.data(), 2, 2 };
    }

    Texture Texture::CreateFromMaterialAsset(const AZ::Data::Asset<AZ::RPI::MaterialAsset>& materialAsset)
    {
        return CreatePlaceholder();
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
