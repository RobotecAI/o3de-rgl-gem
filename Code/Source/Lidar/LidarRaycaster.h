/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

#include <ROS2/Lidar/LidarRaycasterBus.h>

struct Lidar;

namespace RGL
{
    class LidarRaycaster : protected ROS2::LidarRaycasterRequestBus::Handler
    {
    public:
        explicit LidarRaycaster(const AZ::Uuid& uuid);
        LidarRaycaster(LidarRaycaster&& lidarSystem) noexcept;
        LidarRaycaster(const LidarRaycaster& lidarSystem) = delete;
        ~LidarRaycaster() override;

    protected:
        ////////////////////////////////////////////////////////////////////////
        // LidarRaycasterRequestBus::Handler interface implementation
        void ConfigureRays(const AZStd::vector<AZ::Vector3>& rotations, float distance) override;
        void ConfigureNoiseParameters(
            float angularNoiseStdDev, float distanceNoiseStdDevBase, float distanceNoiseStdDevRisePerMeter) override;
        AZStd::vector<AZ::Vector3> PerformRaycast(const AZ::Transform& lidarTransform) override;
        ////////////////////////////////////////////////////////////////////////
    private:
        AZ::Uuid m_uuid;
        Lidar* m_lidar;
    };
} // namespace RGL
