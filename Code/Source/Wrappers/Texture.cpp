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
#include <Wrappers/Texture.h>

namespace RGL::Wrappers
{
    Texture::Texture(const uint8_t* texels, size_t width, size_t height)
    {
        bool success = false;
        Utils::ErrorCheck(
            rgl_texture_create(&m_texture, texels, aznumeric_cast<int32_t>(width), aznumeric_cast<int32_t>(height)),
            __FILE__,
            __LINE__,
            &success);

        if (!success && !m_texture)
        {
            RGL_CHECK(rgl_texture_destroy(m_texture));
            m_texture = nullptr;
        }
    }

    Texture::~Texture()
    {
        if (IsValid())
        {
            RGL_CHECK(rgl_texture_destroy(m_texture));
        }
    }
} // namespace RGL::Wrappers
