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

#include <rgl/api/core.h>

namespace RGL::Wrappers
{
    class Entity;

    class Texture
    {
    public:
        Texture(const uint8_t* texels, size_t width, size_t height);
        ~Texture();

        [[nodiscard]] bool IsValid() const
        {
            return m_texture;
        }

    private:
        rgl_texture_t m_texture{ nullptr };

        friend class Entity;
    };
} // namespace RGL::Wrappers
