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
#include <AzCore/Component/TransformBus.h>
#include <Entity/EntityManager.h>
#include <Mesh/MeshLibraryBus.h>
#include <Utilities/RGLUtils.h>

namespace RGL
{
    EntityManager::EntityManager(AZ::EntityId entityId)
        : m_entityId{ entityId }
    {
        AZ::Render::MeshComponentNotificationBus::Handler::BusConnect(entityId);
        AZ::EntityBus::Handler::BusConnect(entityId);
    }

    EntityManager::EntityManager(EntityManager&& entityManager)
        : m_isStatic{ entityManager.m_isStatic }
        , m_entityId{ entityManager.m_entityId }
        , m_entities{ AZStd::move(entityManager.m_entities) }
    {
        AZ::Render::MeshComponentNotificationBus::Handler::BusConnect(m_entityId);
        AZ::EntityBus::Handler::BusConnect(m_entityId);
    }

    EntityManager::~EntityManager()
    {
        AZ::EntityBus::Handler::BusDisconnect();
        AZ::Render::MeshComponentNotificationBus::Handler::BusDisconnect();

        for (rgl_entity_t entity : m_entities)
        {
            Utils::ErrorCheck(rgl_entity_destroy(entity));
        }
    }

    bool EntityManager::IsStatic() const
    {
        return m_isStatic;
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
            Utils::ErrorCheck(rgl_entity_set_pose(entity, &entityPose));
        }
    }

    void EntityManager::OnModelReady(
        const AZ::Data::Asset<AZ::RPI::ModelAsset>& modelAsset, [[maybe_unused]] const AZ::Data::Instance<AZ::RPI::Model>& model)
    {
        AZ_Assert(m_entities.empty(), "Entity Manager for entity with ID: %s has an invalid state.", m_entityId.ToString().c_str());
        const auto meshes = MeshLibraryInterface::Get()->StoreModelAsset(modelAsset);

        if (meshes.empty())
        {
            AZ_Assert(false, "EntityManager with ID: %s did not receive any mesh from the MeshLibrary.", m_entityId.ToString().c_str());
            return;
        }

        m_entities.reserve(meshes.size());
        for (rgl_mesh_t mesh : meshes)
        {
            rgl_entity_t entity = nullptr;
            Utils::SafeRglEntityCreate(entity, mesh);
            if (entity == nullptr)
            {
                continue;
            }

            m_entities.emplace_back(entity);
        }

        if (!m_entities.empty())
        {
            UpdatePose();
        }
    }

    void EntityManager::OnEntityActivated(const AZ::EntityId& entityId)
    {
        AZ::TransformBus::EventResult(m_isStatic, m_entityId, &AZ::TransformBus::Events::IsStaticTransform);
    }
} // namespace RGL