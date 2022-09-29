/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "LidarRaycaster.h"

#include "RGLBus.h"

namespace RGL
{
    static constexpr rgl_mat3x4f defaultRayTransform = {
        .value{
            { 1, 0, 0, 0 },
            { 0, 1, 0, 0 },
            { 0, 0, 1, 0 },
        },
    };

    LidarRaycaster::LidarRaycaster(const AZ::Uuid& uuid)
        : m_uuid{ uuid }
        , m_lidar{ nullptr }
    {
        ErrorCheck(rgl_lidar_create(&m_lidar, &defaultRayTransform, 1));
        ROS2::LidarRaycasterRequestBus::Handler::BusConnect(uuid);
    }

    LidarRaycaster::LidarRaycaster(LidarRaycaster&& lidarRaycaster) noexcept
        : m_uuid{ lidarRaycaster.m_uuid }
        , m_lidar{ lidarRaycaster.m_lidar }
    {
        lidarRaycaster.m_uuid = AZ::Uuid::CreateNull();
        lidarRaycaster.m_lidar = nullptr;
        lidarRaycaster.BusDisconnect();

        ROS2::LidarRaycasterRequestBus::Handler::BusConnect(m_uuid);
    }

    LidarRaycaster::~LidarRaycaster()
    {
        ROS2::LidarRaycasterRequestBus::Handler::BusDisconnect();

        if (m_lidar != nullptr)
        {
            ErrorCheck(rgl_lidar_destroy(m_lidar));
        }
    }

    void LidarRaycaster::ExcludeEntity(const AZ::EntityId& excludedEntityId)
    {
        RGLInterface::Get()->ExcludeEntity(excludedEntityId);
    }

    void LidarRaycaster::ConfigureRays(const AZStd::vector<AZ::Vector3>& rotations, float distance)
    {
        ErrorCheck(rgl_lidar_destroy(m_lidar));

        std::vector<rgl_mat3x4f> rayTransforms;
        rayTransforms.reserve(rotations.size());
        for (AZ::Vector3 rotation : rotations)
        {
            // Computed by hand since the O3DE built - in conversion don't seem to work as intended.
            const float sinY = AZ::Sin(rotation.GetY() + (AZ::Constants::Pi / 2.0f));
            const float sinZ = AZ::Sin(rotation.GetZ());
            const float cosY = AZ::Cos(rotation.GetY() + (AZ::Constants::Pi / 2.0f));
            const float cosZ = AZ::Cos(rotation.GetZ());

            rayTransforms.push_back({
                {
                    { cosY * cosZ, -sinZ, -(sinY * cosZ), 0.0f },
                    { sinZ * cosY, cosZ, -(sinY * sinZ), 0.0f },
                    { sinY, 0.0f, cosY, 0.0f },
                },
            });
        }

        if (rayTransforms.empty())
        {
            rayTransforms.push_back(defaultRayTransform);
        }

        ErrorCheck(rgl_lidar_create(&m_lidar, rayTransforms.data(), static_cast<int>(rayTransforms.size())));
        ErrorCheck(rgl_lidar_set_range(m_lidar, distance));
    }

    void LidarRaycaster::ConfigureNoiseParameters(
        float angularNoiseStdDev, float distanceNoiseStdDevBase, float distanceNoiseStdDevRisePerMeter)
    {
        ErrorCheck(rgl_lidar_set_gaussian_noise_params(
            m_lidar,
            RGL_ANGULAR_NOISE_TYPE_RAY_BASED, // TODO - Pass angular noise parameters
                                              // (once rotation axis configuration is available in rgl).
            0.0f,
            0.0f,
            distanceNoiseStdDevBase,
            distanceNoiseStdDevRisePerMeter,
            0.0f));
    }

    AZStd::vector<AZ::Vector3> LidarRaycaster::PerformRaycast(const AZ::Transform& lidarTransform)
    {
        AZ::Matrix3x4 lidarPose = AZ::Matrix3x4::CreateFromTransform(lidarTransform);
        rgl_mat3x4f rglLidarPose{
            {
                { lidarPose.GetRow(0).GetX(), lidarPose.GetRow(0).GetY(), lidarPose.GetRow(0).GetZ(), lidarPose.GetRow(0).GetW() },
                { lidarPose.GetRow(1).GetX(), lidarPose.GetRow(1).GetY(), lidarPose.GetRow(1).GetZ(), lidarPose.GetRow(1).GetW() },
                { lidarPose.GetRow(2).GetX(), lidarPose.GetRow(2).GetY(), lidarPose.GetRow(2).GetZ(), lidarPose.GetRow(2).GetW() },
            },
        };

        ErrorCheck(rgl_lidar_set_pose(m_lidar, &rglLidarPose));
        ErrorCheck(rgl_lidar_raytrace_async(nullptr, m_lidar));

        int rglRaycastResultsSize = -1;
        ErrorCheck(rgl_lidar_get_output_size(m_lidar, &rglRaycastResultsSize));
        if (rglRaycastResultsSize <= 0)
        {
            return {};
        }

        AZStd::vector<rgl_vec3f> rglRaycastResults{ static_cast<size_t>(rglRaycastResultsSize) };
        ErrorCheck(rgl_lidar_get_output_data(m_lidar, RGL_FORMAT_XYZ, rglRaycastResults.data()));

        AZStd::vector<AZ::Vector3> raycastResults;
        raycastResults.reserve(rglRaycastResultsSize);
        for (rgl_vec3f point : rglRaycastResults)
        {
            raycastResults.push_back({ point.value[0], point.value[1], point.value[2] });
        }

        return raycastResults;
    }
} // namespace RGL