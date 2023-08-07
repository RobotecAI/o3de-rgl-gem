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

#include <Entity/EntityManager.h>
#include <Utilities/RGLUtils.h>
#include <AzCore/Component/TransformBus.h>

namespace RGL
{
    EntityManager::EntityManager(AZ::EntityId entityId)
        : m_entityId{ entityId }
    {
        AZ::EntityBus::Handler::BusConnect(m_entityId);
    }

    EntityManager::EntityManager(EntityManager&& other)
        : m_entityId{ other.m_entityId }
        , m_entities{ AZStd::move(other.m_entities) }
        , m_isStatic{ other.m_isStatic }
    {
        AZ::EntityBus::Handler::BusConnect(m_entityId);
    }

    EntityManager::~EntityManager()
    {
        AZ::EntityBus::Handler::BusDisconnect();

        for (rgl_entity_t entity : m_entities)
        {
            RGL_CHECK(rgl_entity_destroy(entity));
        }
    }

    void EntityManager::Update()
    {
        if (IsStatic())
        {
            return;
        }

        UpdatePose();
    }

    bool EntityManager::IsStatic() const
    {
        return m_isStatic;
    }

    void EntityManager::OnEntityActivated(const AZ::EntityId& entityId)
    {
        AZ::TransformBus::EventResult(m_isStatic, m_entityId, &AZ::TransformBus::Events::IsStaticTransform);
    }

    void EntityManager::UpdatePose()
    {
        if (m_entities.empty())
        {
            return;
        }

        AZ::Transform transform = AZ::Transform::CreateIdentity();
        AZ::TransformBus::EventResult(transform, m_entityId, &AZ::TransformBus::Events::GetWorldTM);

        const rgl_mat3x4f entityPose = Utils::RglMat3x4FromAzMatrix3x4(AZ::Matrix3x4::CreateFromTransform(transform));

        for (rgl_entity_t entity : m_entities)
        {
            RGL_CHECK(rgl_entity_set_pose(entity, &entityPose));
        }
    }
} // namespace RGL