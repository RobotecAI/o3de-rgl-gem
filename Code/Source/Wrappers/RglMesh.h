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
#pragma once

#include <AzCore/std/containers/vector.h>
#include <rgl/api/core.h>

namespace RGL::Wrappers
{
    class RglEntity;

    class RglMesh
    {
        friend class RglEntity;

    public:
        static RglMesh CreateInvalid()
        {
            return {};
        }

        RglMesh(const rgl_vec3f* vertices, size_t vertexCount, const rgl_vec3i* indices, size_t indexCount);
        RglMesh(const RglMesh& other) = delete;
        RglMesh(RglMesh&& other);
        ~RglMesh();

        [[nodiscard]] bool IsValid() const
        {
            return m_nativePtr;
        }

        void SetTextureCoordinates(const rgl_vec2f* uvs, size_t uvCount);

        RglMesh& operator=(const RglMesh& other) = delete;
        RglMesh& operator=(RglMesh&& other);

    private:
        //! Creates an invalid mesh.
        //! To avoid creating an invalid mesh by accident, it is private.
        //! See CreateInvalid.
        RglMesh() = default;

        rgl_mesh_t m_nativePtr{ nullptr };
    };
} // namespace RGL::Wrappers
