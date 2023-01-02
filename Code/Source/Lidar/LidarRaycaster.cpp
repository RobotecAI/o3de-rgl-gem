/* Copyright 2020-2021, Robotec.ai sp. z o.o.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
        RglUtils::ErrorCheck(rgl_node_rays_from_mat3x4f(&m_rayPosesNode, &RglUtils::IdentityTransform, 1));
        RglUtils::ErrorCheck(rgl_node_rays_transform(&m_lidarTransformNode, &RglUtils::IdentityTransform));
        RglUtils::ErrorCheck(rgl_node_raytrace(&m_rayTraceNode, nullptr, m_range));
        RglUtils::ErrorCheck(rgl_node_points_compact(&m_pointsCompactNode));
        RglUtils::ErrorCheck(
            rgl_node_points_format(&m_pointsFormatNode, DefaultFields.data(), aznumeric_cast<int32_t>(DefaultFields.size())));

        RglUtils::ErrorCheck(rgl_graph_node_add_child(m_rayPosesNode, m_lidarTransformNode));
        RglUtils::ErrorCheck(rgl_graph_node_add_child(m_lidarTransformNode, m_rayTraceNode));
        RglUtils::ErrorCheck(rgl_graph_node_add_child(m_rayTraceNode, m_pointsCompactNode));
        RglUtils::ErrorCheck(rgl_graph_node_add_child(m_pointsCompactNode, m_pointsFormatNode));
    }

    LidarRaycaster::LidarRaycaster(LidarRaycaster&& lidarRaycaster) noexcept
        : m_addMaxRangePoints{ lidarRaycaster.m_addMaxRangePoints }
        , m_uuid{ lidarRaycaster.m_uuid }
        , m_range{ lidarRaycaster.m_range }
        , m_rayTransforms{ AZStd::move(lidarRaycaster.m_rayTransforms) }
        , m_rayPosesNode{ lidarRaycaster.m_rayPosesNode }
        , m_lidarTransformNode{ lidarRaycaster.m_lidarTransformNode }
        , m_rayTraceNode{ lidarRaycaster.m_rayTraceNode }
        , m_pointsCompactNode{ lidarRaycaster.m_pointsCompactNode }
        , m_pointsFormatNode{ lidarRaycaster.m_pointsFormatNode }
    {
        lidarRaycaster.BusDisconnect();

        // Ensure proper destruction of the movee.
        lidarRaycaster.m_uuid = AZ::Uuid::CreateNull();
        lidarRaycaster.m_rayPosesNode = nullptr;

        ROS2::LidarRaycasterRequestBus::Handler::BusConnect(m_uuid);
    }

    LidarRaycaster::~LidarRaycaster()
    {
        if (!m_uuid.IsNull())
        {
            ROS2::LidarRaycasterRequestBus::Handler::BusDisconnect();
        }

        if (m_rayPosesNode != nullptr)
        {
            RglUtils::ErrorCheck(rgl_graph_destroy(m_pointsCompactNode));

            if (m_addMaxRangePoints)
            {
                RglUtils::ErrorCheck(rgl_graph_destroy(m_rayTraceNode));
            }
        }
    }

    void LidarRaycaster::ConfigureRayOrientations(const AZStd::vector<AZ::Vector3>& orientations)
    {
        ValidateRayOrientations(orientations);
        AZStd::vector<rgl_mat3x4f> rglRayTransforms;
        rglRayTransforms.reserve(orientations.size());
        for (AZ::Vector3 orientation : orientations)
        {
            AZ::Matrix3x4 rayTransform = AZ::Matrix3x4::CreateFromQuaternion(AZ::Quaternion::CreateFromEulerRadiansZYX({
                orientation.GetX(),
                -orientation.GetY() + (AZ::Constants::Pi / 2.0f),
                orientation.GetZ() - (AZ::Constants::Pi / 2.0f),
            }));

            rglRayTransforms.push_back(RglUtils::RglMat3x4FromAzMatrix3x4(rayTransform));
        }

        m_rayTransforms.clear();
        m_rayTransforms.reserve(rglRayTransforms.size());
        for (rgl_mat3x4f transform : rglRayTransforms)
        {
            m_rayTransforms.push_back(RglUtils::AzMatrix3x4FromRglMat3x4(transform));
        }

        RglUtils::ErrorCheck(
            rgl_node_rays_from_mat3x4f(&m_rayPosesNode, rglRayTransforms.data(), aznumeric_cast<int32_t>(rglRayTransforms.size())));
    }

    void LidarRaycaster::ConfigureRayRange(float range)
    {
        ValidateRayRange(range);
        m_range = range;
        RglUtils::ErrorCheck(rgl_node_raytrace(&m_rayTraceNode, nullptr, range));
    }

    AZStd::vector<AZ::Vector3> LidarRaycaster::PerformRaycast(const AZ::Transform& lidarTransform)
    {
        AZ::Matrix3x4 lidarPose = AZ::Matrix3x4::CreateFromTransform(lidarTransform);
        rgl_mat3x4f rglLidarPose = RglUtils::RglMat3x4FromAzMatrix3x4(lidarPose);

        RglUtils::ErrorCheck(rgl_node_rays_transform(&m_lidarTransformNode, &rglLidarPose));
        RglUtils::ErrorCheck(rgl_graph_run(m_lidarTransformNode));

        int32_t resultSize = -1;
        RglUtils::ErrorCheck(rgl_graph_get_result_size(m_pointsFormatNode, rgl_field_t::RGL_FIELD_DYNAMIC_FORMAT, &resultSize, nullptr));

        if (resultSize <= 0)
        {
            return {};
        }

        AZStd::vector<DefaultFormatStruct> rglRaycastResults{ aznumeric_cast<size_t>(resultSize) };
        RglUtils::ErrorCheck(
            rgl_graph_get_result_data(m_pointsFormatNode, rgl_field_t::RGL_FIELD_DYNAMIC_FORMAT, rglRaycastResults.data()));

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

    void LidarRaycaster::ExcludeEntities(const AZStd::vector<AZ::EntityId>& excludedEntities)
    {
        for (const auto& entity : excludedEntities)
        {
            RGLInterface::Get()->ExcludeEntity(entity);
        }
    }

    void LidarRaycaster::ConfigureMaxRangePointAddition(bool addMaxRangePoints)
    {
        if (addMaxRangePoints == m_addMaxRangePoints) // Make sure to only allow state switching calls
        {
            return;
        }

        m_addMaxRangePoints = addMaxRangePoints;
        // We need to add or remove the compact node depending on the value of addMaxRangePoints.
        if (addMaxRangePoints)
        {
            RglUtils::ErrorCheck(rgl_graph_node_remove_child(m_rayTraceNode, m_pointsCompactNode));
            RglUtils::ErrorCheck(rgl_graph_node_remove_child(m_pointsCompactNode, m_pointsFormatNode));
            RglUtils::ErrorCheck(rgl_graph_node_add_child(m_rayTraceNode, m_pointsFormatNode));
        }
        else
        {
            RglUtils::ErrorCheck(rgl_graph_node_remove_child(m_rayTraceNode, m_pointsFormatNode));
            RglUtils::ErrorCheck(rgl_graph_node_add_child(m_rayTraceNode, m_pointsCompactNode));
            RglUtils::ErrorCheck(rgl_graph_node_add_child(m_pointsCompactNode, m_pointsFormatNode));
        }
    }

    const AZStd::vector<rgl_field_t> LidarRaycaster::DefaultFields{ RGL_FIELD_XYZ_F32, RGL_FIELD_IS_HIT_I32 };
} // namespace RGL