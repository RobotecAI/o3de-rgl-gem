/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzFramework/Terrain/TerrainDataRequestBus.h>
#include <AzFramework/Visibility/BoundsBus.h>
#include <rgl/api/core.h>
#include <AzCore/Component/Component.h>

namespace RGL
{
    //! Queries the TerrainDataRequestBus for terrain heights and constructs a mesh using them.
    //! Terrain area is split into square sectors of predetermined width.
    //! The constructed mesh has a uniform vertex distribution along the xy - plane.
    class TerrainEntityManagerSystemComponent
        : public AZ::Component
        , private AzFramework::Terrain::TerrainDataNotificationBus::Handler
    {
    public:
        AZ_COMPONENT(TerrainEntityManagerSystemComponent, "{6de4556f-5621-4ec3-a587-b28988f79d8a}");
        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        TerrainEntityManagerSystemComponent() = default;
        ~TerrainEntityManagerSystemComponent();

        ////////////////////////////////////////////////////////////////////////
        // AzFramework::Terrain::TerrainDataNotificationBus interface implementation
        void OnTerrainDataChanged(const AZ::Aabb& dirtyRegion, TerrainDataChangedMask dataChangedMask) override;
        ////////////////////////////////////////////////////////////////////////
    protected:
        void Init() override;
        void Activate() override;
        void Deactivate() override;

    private:
        void EnsureManagedEntityDestroyed();

        void UpdateWorldBounds();
        void UpdateDirtyRegion(const AZ::Aabb& dirtyRegion);

        rgl_mesh_t m_rglMesh{ nullptr };
        rgl_entity_t m_rglEntity{ nullptr };

        AZ::Aabb m_currentWorldBounds = AZ::Aabb::CreateFromPoint(AZ::Vector3::CreateZero());
        AZStd::vector<rgl_vec3f> m_vertices;
        AZStd::vector<rgl_vec3i> m_indices;

        static constexpr size_t TrianglesPerSector = 2LU;
        // Length of each sector side (in meters);
        static constexpr float SectorSideLength = 1.0f;
    };
} // namespace ROS2
