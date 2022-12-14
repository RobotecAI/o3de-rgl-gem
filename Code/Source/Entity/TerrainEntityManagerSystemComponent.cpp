/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "TerrainEntityManagerSystemComponent.h"
#include "Utilities/RGLUtils.h"
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/EditContextConstants.inl>
#include <AzCore/Serialization/SerializeContext.h>

namespace RGL
{
    TerrainEntityManagerSystemComponent::~TerrainEntityManagerSystemComponent()
    {
        EnsureManagedEntityDestroyed();
    }

    void TerrainEntityManagerSystemComponent::Init()
    {
    }

    void TerrainEntityManagerSystemComponent::Activate()
    {
        UpdateWorldBounds();
        AzFramework::Terrain::TerrainDataNotificationBus::Handler::BusConnect();
    }

    void TerrainEntityManagerSystemComponent::Deactivate()
    {
        AzFramework::Terrain::TerrainDataNotificationBus::Handler::BusDisconnect();
    }

    void TerrainEntityManagerSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<TerrainEntityManagerSystemComponent, AZ::Component>()->Version(0);

            if (AZ::EditContext* ec = serializeContext->GetEditContext())
            {
                ec->Class<TerrainEntityManagerSystemComponent>("Terrain Entity Manager", "Manages the RGL Terrain entity.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
                    ->Attribute(AZ::Edit::Attributes::Category, "RGL")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true);
            }
        }
    }

    void TerrainEntityManagerSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("TerrainEntityManagerService"));
    }

    void TerrainEntityManagerSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("TerrainEntityManagerService"));
    }

    void TerrainEntityManagerSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("RGLService"));
    }

    void TerrainEntityManagerSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    void TerrainEntityManagerSystemComponent::EnsureManagedEntityDestroyed()
    {
        if (m_rglEntity != nullptr)
        {
            RglUtils::ErrorCheck(rgl_entity_destroy(m_rglEntity));
            m_rglEntity = nullptr;
        }

        if (m_rglMesh != nullptr)
        {
            RglUtils::ErrorCheck(rgl_mesh_destroy(m_rglMesh));
            m_rglMesh = nullptr;
        }
    }

    void TerrainEntityManagerSystemComponent::UpdateWorldBounds()
    {
        AZ::Aabb newWorldBounds = AZ::Aabb::CreateFromPoint(AZ::Vector3::CreateZero());
        AzFramework::Terrain::TerrainDataRequestBus::BroadcastResult(
            newWorldBounds, &AzFramework::Terrain::TerrainDataRequests::GetTerrainAabb);

        const AZ::Vector3 newWorldMin = newWorldBounds.GetMin();
        const AZ::Vector3 newWorldSize = newWorldBounds.GetExtents();

        if (newWorldBounds == m_currentWorldBounds || newWorldSize.GetX() < SectorSideLength || newWorldSize.GetY() < SectorSideLength)
        {
            return; // No need to update.
        }

        m_currentWorldBounds = newWorldBounds;
        size_t sectorCountX = aznumeric_cast<size_t>(newWorldSize.GetX() / SectorSideLength);
        size_t sectorCountY = aznumeric_cast<size_t>(newWorldSize.GetY() / SectorSideLength);

        // To create a mesh constructed out of n * m sectors we need (n + 1) * (m + 1) vertices.
        // Each vertex is positioned in one of the sector's corners.
        size_t vertexCountX = sectorCountX + 1LU;
        size_t vertexCountY = sectorCountY + 1LU;

        m_vertices.clear();
        m_vertices.reserve(vertexCountX * vertexCountY);
        for (size_t vertexIndexX = 0LU; vertexIndexX < vertexCountX; ++vertexIndexX)
        {
            for (size_t vertexIndexY = 0LU; vertexIndexY < vertexCountY; ++vertexIndexY)
            {
                m_vertices.emplace_back(rgl_vec3f{
                    newWorldMin.GetX() + aznumeric_cast<float>(vertexIndexX) * SectorSideLength,
                    newWorldMin.GetY() + aznumeric_cast<float>(vertexIndexY) * SectorSideLength,
                    0.0f,
                });
            }
        }

        m_indices.clear();
        m_indices.reserve(sectorCountX * sectorCountY * TrianglesPerSector);
        for (size_t sectorIndexX = 0LU; sectorIndexX < sectorCountX; ++sectorIndexX)
        {
            for (size_t sectorIndexY = 0LU; sectorIndexY < sectorCountY; ++sectorIndexY)
            {
                const int32_t upperLeft = aznumeric_cast<int32_t>(sectorIndexY * vertexCountX + sectorIndexX);
                const int32_t upperRight = aznumeric_cast<int32_t>(upperLeft + 1LU);
                const int32_t lowerLeft = aznumeric_cast<int32_t>(upperLeft + vertexCountX);
                const int32_t lowerRight = aznumeric_cast<int32_t>(upperRight + vertexCountX);

                m_indices.emplace_back(rgl_vec3i{ upperLeft, upperRight, lowerLeft });
                m_indices.emplace_back(rgl_vec3i{ lowerLeft, upperRight, lowerRight });
            }
        }

        EnsureManagedEntityDestroyed();

        RglUtils::ErrorCheck(rgl_mesh_create(
            &m_rglMesh,
            m_vertices.data(),
            aznumeric_cast<int32_t>(m_vertices.size()),
            m_indices.data(),
            aznumeric_cast<int32_t>(m_indices.size())));

        RglUtils::ErrorCheck(rgl_entity_create(&m_rglEntity, nullptr, m_rglMesh));
        RglUtils::ErrorCheck(rgl_entity_set_pose(m_rglEntity, &RglUtils::IdentityTransform));
    }

    void TerrainEntityManagerSystemComponent::UpdateDirtyRegion(const AZ::Aabb& dirtyRegion)
    {
        const AZ::Aabb dirtyRegion2D = AZ::Aabb::CreateFromMinMaxValues(
            dirtyRegion.GetMin().GetX(), dirtyRegion.GetMin().GetY(), AZStd::numeric_limits<float>::lowest(),
            dirtyRegion.GetMax().GetX(), dirtyRegion.GetMax().GetY(), AZStd::numeric_limits<float>::max());

        for (rgl_vec3f& vertex : m_vertices)
        {
            if (dirtyRegion2D.Contains(RglUtils::AzVector3FromRglVec3f(vertex)))
            {
                float height = AZStd::numeric_limits<float>::lowest();
                bool terrainExists = false;
                AzFramework::Terrain::TerrainDataRequestBus::BroadcastResult(
                    height,
                    &AzFramework::Terrain::TerrainDataRequestBus::Events::GetHeightFromFloats,
                    vertex.value[0],
                    vertex.value[1],
                    AzFramework::Terrain::TerrainDataRequests::Sampler::BILINEAR,
                    &terrainExists);

                if (height != AZStd::numeric_limits<float>::lowest() && terrainExists)
                {
                    // TODO - Make sure that the vertex' z value is set properly (depending on rotation)
                    vertex.value[2] = height;
                }
            }
        }

        RglUtils::ErrorCheck(rgl_mesh_update_vertices(m_rglMesh, m_vertices.data(), aznumeric_cast<int32_t>(m_vertices.size())));
    }

    void TerrainEntityManagerSystemComponent::OnTerrainDataChanged(const AZ::Aabb& dirtyRegion, TerrainDataChangedMask dataChangedMask)
    {
        if (dataChangedMask & TerrainDataChangedMask::Settings)
        {
            UpdateWorldBounds();
        }

        if (dataChangedMask & TerrainDataChangedMask::HeightData)
        {
            UpdateDirtyRegion(dirtyRegion);
        }
    }
} // namespace ROS2
