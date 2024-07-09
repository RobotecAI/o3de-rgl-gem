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

#include <AzCore/std/containers/vector.h>
#include <rgl/api/core.h>

namespace RGL::Wrappers
{
    class Entity;

    class Mesh
    {
    public:
        Mesh(const rgl_vec3f* vertices, size_t vertexCount, const rgl_vec3i* indices, size_t indexCount);
        Mesh(const Mesh& other) = delete;
        Mesh(Mesh&& other);
        ~Mesh();

        [[nodiscard]] bool IsValid() const
        {
            return m_mesh;
        }

        void UpdateVertices(const rgl_vec3f* vertices, size_t vertexCount);
        void SetTextureCoordinates(const rgl_vec2f* uvs, size_t uvCount);

        Mesh& operator=(const Mesh& other) = delete;
        Mesh& operator=(Mesh&& other);

    private:
        rgl_mesh_t m_mesh{ nullptr };

        friend class Entity;
    };
} // namespace RGL::Wrappers
