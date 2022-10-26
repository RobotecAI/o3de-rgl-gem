/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "EntityManager.h"
#include "Mesh/MeshLibraryBus.h"

namespace RGL
{
    EntityManager::EntityManager(AZ::EntityId entityId)
        : m_isStatic{ false }
        , m_entityId{ entityId }
    {
        AZ::Render::MeshComponentNotificationBus::Handler::BusConnect(entityId);
        AZ::EntityBus::Handler::BusConnect(entityId);
    }

    EntityManager::~EntityManager()
    {
        AZ::EntityBus::Handler::BusDisconnect();
        AZ::Render::MeshComponentNotificationBus::Handler::BusDisconnect();

        for (rgl_entity_t entity : m_entities)
        {
            ErrorCheck(rgl_entity_destroy(entity));
        }
    }

    bool EntityManager::IsStatic()
    {
        return m_isStatic;
    }

    void EntityManager::UpdatePose()
    {
        AZ::Transform transform = AZ::Transform::CreateIdentity();
        AZ::TransformBus::EventResult(transform, m_entityId, &AZ::TransformBus::Events::GetWorldTM);
        AZ::Matrix4x4 matrix = AZ::Matrix4x4::CreateFromTransform(transform);

        rgl_mat3x4f entityPose = {
            {
                { matrix.GetRow(0).GetX(), matrix.GetRow(0).GetY(), matrix.GetRow(0).GetZ(), matrix.GetRow(0).GetW() },
                { matrix.GetRow(1).GetX(), matrix.GetRow(1).GetY(), matrix.GetRow(1).GetZ(), matrix.GetRow(1).GetW() },
                { matrix.GetRow(2).GetX(), matrix.GetRow(2).GetY(), matrix.GetRow(2).GetZ(), matrix.GetRow(2).GetW() },
            },
        };

        for (rgl_entity_t entity : m_entities)
        {
            ErrorCheck(rgl_entity_set_pose(entity, &entityPose));
        }
    }

    void EntityManager::OnModelReady(
        const AZ::Data::Asset<AZ::RPI::ModelAsset>& modelAsset, [[maybe_unused]] const AZ::Data::Instance<AZ::RPI::Model>& model)
    {
        AZ_Assert(m_entities.empty(), "Entity Manager has an invalid state.");
        auto meshes = MeshLibraryInterface::Get()->GetMeshPointers(modelAsset);

        m_entities.reserve(meshes.size());
        for (Mesh* meshPtr : meshes)
        {
            rgl_entity_t entity = nullptr;
            ErrorCheck(rgl_entity_create(&entity, nullptr, meshPtr));

            m_entities.emplace_back(entity);
        }

        UpdatePose();
    }

    void EntityManager::OnEntityActivated(const AZ::EntityId& entityId)
    {
        AZ::TransformBus::EventResult(m_isStatic, m_entityId, &AZ::TransformBus::Events::IsStaticTransform);
    }
} // namespace RGL