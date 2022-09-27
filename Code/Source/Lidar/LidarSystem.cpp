/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#include "LidarSystem.h"

namespace RGL
{
    void LidarSystem::Activate(int handlerId)
    {
        ROS2::LidarSystemRequestBus::Handler::BusConnect(handlerId);
    }

    void LidarSystem::Deactivate()
    {
        ROS2::LidarSystemRequestBus::Handler::BusDisconnect();
    }

    AZ::Uuid LidarSystem::CreateLidar([[maybe_unused]] const AZ::EntityId& lidarEntityId)
    {
        AZ::Uuid lidarUuid = AZ::Uuid::CreateRandom();
        m_lidars.emplace_back(lidarUuid);
        return lidarUuid;
    }

    bool LidarSystem::IsNoiseSupported()
    {
        return true;
    }
} // namespace RGL