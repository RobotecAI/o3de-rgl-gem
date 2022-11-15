/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "LidarRaycaster.h"

#include "Utilities/RGLUtils.h"

#include "RGLBus.h"

namespace RGL
{
    LidarRaycaster::LidarRaycaster(const AZ::Uuid& uuid)
        : m_uuid{ uuid }
    {
        ROS2::LidarRaycasterRequestBus::Handler::BusConnect(uuid);

        // Configure the default graph
        RglUtils::ErrorCheck(rgl_node_rays_from_mat3x4f(&m_rayPoses, &IdentityTransform, 1));
        RglUtils::ErrorCheck(rgl_node_rays_transform(&m_lidarTransform, &IdentityTransform));
        RglUtils::ErrorCheck(rgl_node_raytrace(&m_rayTrace, nullptr, m_range));
        RglUtils::ErrorCheck(rgl_node_points_compact(&m_pointsCompact));
        RglUtils::ErrorCheck(
            rgl_node_points_format(&m_pointsFormat, DefaultFields.data(), aznumeric_cast<int32_t>(DefaultFields.size())));

        RglUtils::ErrorCheck(rgl_graph_node_add_child(m_rayPoses, m_lidarTransform));
        RglUtils::ErrorCheck(rgl_graph_node_add_child(m_lidarTransform, m_rayTrace));
        RglUtils::ErrorCheck(rgl_graph_node_add_child(m_rayTrace, m_pointsCompact));
        RglUtils::ErrorCheck(rgl_graph_node_add_child(m_pointsCompact, m_pointsFormat));
    }

    LidarRaycaster::LidarRaycaster(LidarRaycaster&& lidarRaycaster) noexcept
        : m_addMaxRangePoints{ lidarRaycaster.m_addMaxRangePoints }
        , m_uuid{ lidarRaycaster.m_uuid }
        , m_range{ lidarRaycaster.m_range }
        , m_rayTransforms{ AZStd::move(lidarRaycaster.m_rayTransforms) }
        , m_rayPoses{ lidarRaycaster.m_rayPoses }
        , m_lidarTransform{ lidarRaycaster.m_lidarTransform }
        , m_rayTrace{ lidarRaycaster.m_rayTrace }
        , m_pointsCompact{ lidarRaycaster.m_pointsCompact }
        , m_pointsFormat{ lidarRaycaster.m_pointsFormat }
    {
        lidarRaycaster.BusDisconnect();

        // Ensure proper destruction of the movee.
        lidarRaycaster.m_uuid = AZ::Uuid::CreateNull();
        lidarRaycaster.m_rayPoses = nullptr;

        ROS2::LidarRaycasterRequestBus::Handler::BusConnect(m_uuid);
    }

    LidarRaycaster::~LidarRaycaster()
    {
        if (!m_uuid.IsNull())
        {
            ROS2::LidarRaycasterRequestBus::Handler::BusDisconnect();
        }

        if (m_rayPoses != nullptr)
        {
            RglUtils::ErrorCheck(rgl_graph_destroy(m_pointsCompact));

            if (m_addMaxRangePoints)
            {
                rgl_graph_destroy(m_rayTrace);
            }
        }
    }

    void LidarRaycaster::ConfigureRayOrientations(const AZStd::vector<AZ::Vector3>& orientations)
    {
        ValidateRayOrientations(orientations);
        AZStd::vector<rgl_mat3x4f> rglRayTransforms;
        rglRayTransforms.reserve(orientations.size());
        for (AZ::Vector3 rotation : orientations)
        {
            // Computed by hand since the O3DE built - in conversion don't seem to work as intended.
            const float Y = -rotation.GetY() + (AZ::Constants::Pi / 2.0f);
            const float Z = rotation.GetZ() + (AZ::Constants::Pi / 2.0f);

            const float SinY = AZ::Sin(Y);
            const float SinZ = AZ::Sin(Z);
            const float CosY = AZ::Cos(Y);
            const float CosZ = AZ::Cos(Z);

            rglRayTransforms.push_back({
                {
                    { CosY * CosZ, -SinZ, -(SinY * CosZ), 0.0f },
                    { SinZ * CosY, CosZ, -(SinY * SinZ), 0.0f },
                    { SinY, 0.0f, CosY, 0.0f },
                },
            });
        }

        m_rayTransforms.clear();
        m_rayTransforms.reserve(rglRayTransforms.size());
        for (rgl_mat3x4f transform : rglRayTransforms)
        {
            float m_rowMajorValues[]{
                transform.value[0][0], transform.value[0][1], transform.value[0][2], transform.value[0][3],
                transform.value[1][0], transform.value[1][1], transform.value[1][2], transform.value[1][3],
                transform.value[2][0], transform.value[2][1], transform.value[2][2], transform.value[2][3],
            };
            m_rayTransforms.push_back(AZ::Matrix3x4::CreateFromRowMajorFloat12(m_rowMajorValues));
        }

        RglUtils::ErrorCheck(
            rgl_node_rays_from_mat3x4f(&m_rayPoses, rglRayTransforms.data(), aznumeric_cast<int32_t>(rglRayTransforms.size())));
    }

    void LidarRaycaster::ConfigureRayRange(float range)
    {
        ValidateRayRange(range);
        m_range = range;
        RglUtils::ErrorCheck(rgl_node_raytrace(&m_rayTrace, nullptr, range));
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

        RglUtils::ErrorCheck(rgl_node_rays_transform(&m_lidarTransform, &rglLidarPose));
        RglUtils::ErrorCheck(rgl_graph_run(m_lidarTransform));

        int32_t resultSize = -1;
        RglUtils::ErrorCheck(rgl_graph_get_result_size(m_pointsFormat, rgl_field_t::RGL_FIELD_DYNAMIC_FORMAT, &resultSize, nullptr));

        if (resultSize <= 0)
        {
            return {};
        }

        AZStd::vector<DefaultFormatStruct> rglRaycastResults{ aznumeric_cast<size_t>(resultSize) };
        RglUtils::ErrorCheck(rgl_graph_get_result_data(m_pointsFormat, rgl_field_t::RGL_FIELD_DYNAMIC_FORMAT, rglRaycastResults.data()));

        AZStd::vector<AZ::Vector3> raycastResults;
        raycastResults.reserve(resultSize);
        for (size_t result_index = 0LU; result_index < rglRaycastResults.size(); ++result_index)
        {
            if (m_addMaxRangePoints && (rglRaycastResults[result_index].m_isHit == 0))
            {
                AZ::Vector4 maxVector = lidarPose * m_rayTransforms[result_index] * AZ::Vector4(0.0f, 0.0f, m_range, 1.0f);
                raycastResults.push_back(maxVector.GetAsVector3());
            }
            else
            {
                raycastResults.push_back({ rglRaycastResults[result_index].m_xyz.value[0],
                                           rglRaycastResults[result_index].m_xyz.value[1],
                                           rglRaycastResults[result_index].m_xyz.value[2] });
            }
        }

        return raycastResults;
    }

    void LidarRaycaster::ConfigureMaxRangePointAddition(bool addMaxRangePoints)
    {
        if (addMaxRangePoints == m_addMaxRangePoints) // Make sure to only allow state switching calls
        {
            return;
        }

        m_addMaxRangePoints = addMaxRangePoints;
        if (addMaxRangePoints)
        {
            RglUtils::ErrorCheck(rgl_graph_node_remove_child(m_rayTrace, m_pointsCompact));
            RglUtils::ErrorCheck(rgl_graph_node_remove_child(m_pointsCompact, m_pointsFormat));
            RglUtils::ErrorCheck(rgl_graph_node_add_child(m_rayTrace, m_pointsFormat));
        }
        else
        {
            RglUtils::ErrorCheck(rgl_graph_node_remove_child(m_rayTrace, m_pointsFormat));
            RglUtils::ErrorCheck(rgl_graph_node_add_child(m_rayTrace, m_pointsCompact));
            RglUtils::ErrorCheck(rgl_graph_node_add_child(m_pointsCompact, m_pointsFormat));
        }
    }

    const AZStd::vector<rgl_field_t> LidarRaycaster::DefaultFields{ RGL_FIELD_XYZ_F32, RGL_FIELD_IS_HIT_I32 };
} // namespace RGL