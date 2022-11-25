/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

#include <ROS2/Lidar/LidarRaycasterBus.h>
#include <rgl/api/core.h>

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
        void ConfigureRayOrientations(const AZStd::vector<AZ::Vector3>& orientations) override;
        void ConfigureRayRange(float range) override;
        AZStd::vector<AZ::Vector3> PerformRaycast(const AZ::Transform& lidarTransform) override;
        void ConfigureMaxRangePointAddition(bool addMaxRangePoints) override;
        ////////////////////////////////////////////////////////////////////////
    private:
        //! Structure used for rgl-generated output retrieval.
        struct DefaultFormatStruct
        {
            rgl_vec3f m_xyz;
            int32_t m_isHit;
        };

        static const AZStd::vector<rgl_field_t> DefaultFields;

        bool m_addMaxRangePoints{ false };
        AZ::Uuid m_uuid;
        float m_range{ 1.0f };
        AZStd::vector<AZ::Matrix3x4> m_rayTransforms{ AZ::Matrix3x4::CreateIdentity() };
        rgl_node_t m_rayPosesNode{ nullptr }, m_lidarTransformNode{ nullptr }, m_rayTraceNode{ nullptr }, m_pointsCompactNode{ nullptr },
            m_pointsFormatNode{ nullptr };
    };
} // namespace RGL
