/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "EntityManager.h"

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

        for (auto mesh = m_meshes.begin(); mesh != m_meshes.end(); ++mesh)
        {
            ErrorCheck(rgl_mesh_destroy(mesh->second));
            ErrorCheck(rgl_entity_destroy(mesh->first));
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

        for (auto mesh = m_meshes.begin(); mesh != m_meshes.end(); ++mesh)
        {
            ErrorCheck(rgl_entity_set_pose(mesh->first, &entityPose));
        }
    }

    void EntityManager::OnModelReady(
        const AZ::Data::Asset<AZ::RPI::ModelAsset>& modelAsset, [[maybe_unused]] const AZ::Data::Instance<AZ::RPI::Model>& model)
    {
        auto lodAssets = modelAsset->GetLodAssets();
        auto modelLodAsset = lodAssets.begin()->Get();

        auto meshes = modelLodAsset->GetMeshes();
        m_meshes.reserve(meshes.size());
        for (auto mesh = meshes.begin(); mesh != meshes.end(); ++mesh)
        {
            AZStd::pair<rgl_entity_t, rgl_mesh_t> meshEntry;

            AZStd::span<const rgl_vec3f> vertices = mesh->GetSemanticBufferTyped<rgl_vec3f>(AZ::Name("POSITION"));
            AZStd::span<const rgl_vec3i> indices = mesh->GetIndexBufferTyped<rgl_vec3i>();

            ErrorCheck(rgl_mesh_create(&meshEntry.second, vertices.data(), vertices.size(), indices.data(), indices.size()));
            ErrorCheck(rgl_entity_create(&meshEntry.first, nullptr, meshEntry.second));

            m_meshes.emplace_back(std::move(meshEntry));
        }

        UpdatePose();
    }

    void EntityManager::OnEntityActivated(const AZ::EntityId& entityId)
    {
        AZ::TransformBus::EventResult(m_isStatic, m_entityId, &AZ::TransformBus::Events::IsStaticTransform);
    }
} // namespace RGL