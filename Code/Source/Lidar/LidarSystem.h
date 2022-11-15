/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

#include "LidarRaycaster.h"
#include "ROS2/Lidar/LidarSystemBus.h"

namespace RGL
{
    class LidarSystem : protected ROS2::LidarSystemRequestBus::Handler
    {
    public:
        LidarSystem() = default;
        LidarSystem(LidarSystem&& lidarSystem) = delete;
        LidarSystem(const LidarSystem& lidarSystem) = delete;
        ~LidarSystem() = default;

        void Activate(int handlerId);
        void Deactivate();

    protected:
        ////////////////////////////////////////////////////////////////////////
        // LidarSystemRequestBus::Handler interface implementation
        AZ::Uuid CreateLidar(const AZ::EntityId& lidarEntityId) override;
        ROS2::LidarImplementationFeatures GetSupportedFeatures() override;
        ////////////////////////////////////////////////////////////////////////
    private:
        AZStd::vector<LidarRaycaster> m_lidars;
    };
} // namespace RGL
