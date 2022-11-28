/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#include "LidarSystem.h"
#include "RGLBus.h"
#include <ROS2/Lidar/LidarRegistrarBus.h>

namespace RGL
{
    void LidarSystem::Activate()
    {
        const char* name = "RobotecGPULidar";
        const char* description = "Mesh-based lidar implementation that uses the RobotecGPULidar API for GPU-enabled raycasting.";
        const ROS2::LidarSystemFeatures supportedFeatures = {
            /* .m_noise =                   */ false,
            /* .m_collisionLayers =         */ false,
            /* .m_entityExclusion =         */ true,
            /* .m_MaxRangeHitPointConfig =  */ true,
        };

        ROS2::LidarSystemRequestBus::Handler::BusConnect(AZ_CRC(name));

        auto* lidarSystemManagerInterface = ROS2::LidarRegistrarInterface::Get();
        AZ_Assert(lidarSystemManagerInterface != nullptr, "The ROS2 LidarSystem Manager interface was inaccessible.");
        lidarSystemManagerInterface->RegisterLidarSystem(name, description, supportedFeatures);
    }

    void LidarSystem::Deactivate()
    {
        ROS2::LidarSystemRequestBus::Handler::BusDisconnect();
    }

    void LidarSystem::Clear()
    {
        m_lidars.clear();
    }

    AZ::Uuid LidarSystem::CreateLidar(const AZ::EntityId& lidarEntityId)
    {
        AZ::Uuid lidarUuid = AZ::Uuid::CreateRandom();
        m_lidars.emplace_back(lidarUuid);
        return lidarUuid;
    }
} // namespace RGL