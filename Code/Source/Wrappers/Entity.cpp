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
#include <Utilities/RGLUtils.h>
#include <Wrappers/Entity.h>
#include <Wrappers/Mesh.h>
#include <Wrappers/Texture.h>

namespace RGL::Wrappers
{
    Entity::Entity(const Mesh& mesh)
    {
        bool success = false;
        Utils::ErrorCheck(rgl_entity_create(&m_nativePtr, nullptr, mesh.m_nativePtr), __FILE__, __LINE__, &success);
        if (!success && m_nativePtr)
        {
            RGL_CHECK(rgl_entity_destroy(m_nativePtr));
            m_nativePtr = nullptr;
        }
    }

    Entity::Entity(Entity&& other)
        : m_nativePtr(other.m_nativePtr)
    {
        other.m_nativePtr = nullptr;
    }

    Entity::~Entity()
    {
        if (IsValid())
        {
            RGL_CHECK(rgl_entity_destroy(m_nativePtr));
        }
    }

    void Entity::SetPose(const rgl_mat3x4f& pose)
    {
        AZ_Assert(IsValid(), "Tried to set pose of an invalid entity.");
        RGL_CHECK(rgl_entity_set_pose(m_nativePtr, &pose));
    }

    void Entity::SetId(int32_t id)
    {
        AZ_Assert(IsValid(), "Tried to set id of an invalid entity.");
        RGL_CHECK(rgl_entity_set_id(m_nativePtr, id));
    }

    void Entity::SetIntensityTexture(const Texture& texture)
    {
        AZ_Assert(IsValid(), "Tried to set intensity texture of an invalid entity.");
        RGL_CHECK(rgl_entity_set_intensity_texture(m_nativePtr, texture.m_nativePtr));
    }

    Entity& Entity::operator=(Entity&& other)
    {
        if (this != &other)
        {
            if (IsValid())
            {
                RGL_CHECK(rgl_entity_destroy(m_nativePtr));
            }

            m_nativePtr = other.m_nativePtr;
            other.m_nativePtr = nullptr;
        }

        return *this;
    }
} // namespace RGL::Wrappers
