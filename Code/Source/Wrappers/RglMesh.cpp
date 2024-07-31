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
#include <Utilities/RGLUtils.h>
#include <Wrappers/RglMesh.h>
#include <rgl/api/core.h>

namespace RGL::Wrappers
{
    RglMesh::RglMesh(const rgl_vec3f* vertices, size_t vertexCount, const rgl_vec3i* indices, size_t indexCount)
    {
        bool success = false;
        Utils::ErrorCheck(
            rgl_mesh_create(&m_nativePtr, vertices, aznumeric_cast<int32_t>(vertexCount), indices, aznumeric_cast<int32_t>(indexCount)),
            __FILE__,
            __LINE__,
            &success);

        if (!success && m_nativePtr)
        {
            RGL_CHECK(rgl_mesh_destroy(m_nativePtr));
            m_nativePtr = nullptr;
        }
    }

    RglMesh::RglMesh(RglMesh&& other)
    {
        if (IsValid())
        {
            RGL_CHECK(rgl_mesh_destroy(m_nativePtr));
        }

        m_nativePtr = other.m_nativePtr;
        other.m_nativePtr = nullptr;
    }

    RglMesh::~RglMesh()
    {
        if (IsValid())
        {
            RGL_CHECK(rgl_mesh_destroy(m_nativePtr));
            m_nativePtr = nullptr;
        }
    }

    void RglMesh::UpdateVertices(const rgl_vec3f* vertices, size_t vertexCount)
    {
        AZ_Assert(IsValid(), "Tried to update vertices of an invalid mesh.");
        RGL_CHECK(rgl_mesh_update_vertices(m_nativePtr, vertices, aznumeric_cast<int32_t>(vertexCount)));
    }

    void RglMesh::SetTextureCoordinates(const rgl_vec2f* uvs, size_t uvCount)
    {
        AZ_Assert(IsValid(), "Tried to set texture coordinates of an invalid mesh.");
        RGL_CHECK(rgl_mesh_set_texture_coords(m_nativePtr, uvs, aznumeric_cast<int32_t>(uvCount)));
    }

    RglMesh& RglMesh::operator=(RglMesh&& other)
    {
        if (this != &other)
        {
            if (IsValid())
            {
                RGL_CHECK(rgl_mesh_destroy(m_nativePtr));
            }

            m_nativePtr = other.m_nativePtr;
            other.m_nativePtr = nullptr;
        }

        return *this;
    }
} // namespace RGL::Wrappers
