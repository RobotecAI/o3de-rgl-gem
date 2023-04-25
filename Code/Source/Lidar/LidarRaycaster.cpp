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
#include <Lidar/LidarRaycaster.h>
#include <RGL/RGLBus.h>
#include <Utilities/RGLUtils.h>

namespace RGL
{
    LidarRaycaster::LidarRaycaster(const AZ::Uuid& uuid)
        : m_uuid{ uuid }
        , m_rglRaycastResults{ m_resultFields, 1LU }
    {
        ROS2::LidarRaycasterRequestBus::Handler::BusConnect(ROS2::LidarId(uuid));

        // Configure the default graph
        Utils::ErrorCheck(rgl_node_rays_from_mat3x4f(&m_rayPosesNode, &Utils::IdentityTransform, 1));
        Utils::ErrorCheck(rgl_node_rays_transform(&m_lidarTransformNode, &Utils::IdentityTransform));
        Utils::ErrorCheck(rgl_node_raytrace(&m_rayTraceNode, nullptr, m_range));
        Utils::ErrorCheck(rgl_node_points_compact(&m_pointsCompactNode));
        Utils::ErrorCheck(
            rgl_node_points_format(&m_pointsFormatNode, m_resultFields.data(), aznumeric_cast<int32_t>(m_resultFields.size())));
        Utils::ErrorCheck(rgl_graph_node_add_child(m_rayPosesNode, m_lidarTransformNode));
        Utils::ErrorCheck(rgl_graph_node_add_child(m_lidarTransformNode, m_rayTraceNode));
        Utils::ErrorCheck(rgl_graph_node_add_child(m_rayTraceNode, m_pointsCompactNode));
        Utils::ErrorCheck(rgl_graph_node_add_child(m_pointsCompactNode, m_pointsFormatNode));
    }

    LidarRaycaster::LidarRaycaster(LidarRaycaster&& lidarRaycaster) noexcept
        : m_addMaxRangePoints{ lidarRaycaster.m_addMaxRangePoints }
        , m_uuid{ lidarRaycaster.m_uuid }
        , m_range{ lidarRaycaster.m_range }
        , m_minRange{ lidarRaycaster.m_minRange }
        , m_resultFlags{ AZStd::move(lidarRaycaster.m_resultFlags) }
        , m_resultFields{ AZStd::move(lidarRaycaster.m_resultFields) }
        , m_rayTransforms{ AZStd::move(lidarRaycaster.m_rayTransforms) }
        , m_rglRaycastResults{ AZStd::move(lidarRaycaster.m_rglRaycastResults) }
        , m_rayPosesNode{ AZStd::move(lidarRaycaster.m_rayPosesNode) }
        , m_lidarTransformNode{ AZStd::move(lidarRaycaster.m_lidarTransformNode) }
        , m_rayTraceNode{ AZStd::move(lidarRaycaster.m_rayTraceNode) }
        , m_pointsCompactNode{ AZStd::move(lidarRaycaster.m_pointsCompactNode) }
        , m_pointsFormatNode{ AZStd::move(lidarRaycaster.m_pointsFormatNode) }
    {
        lidarRaycaster.BusDisconnect();

        // Ensure proper destruction of the movee.
        lidarRaycaster.m_uuid = AZ::Uuid::CreateNull();
        lidarRaycaster.m_rayPosesNode = nullptr;
        ROS2::LidarRaycasterRequestBus::Handler::BusConnect(ROS2::LidarId(m_uuid));
    }

    LidarRaycaster::~LidarRaycaster()
    {
        if (!m_uuid.IsNull())
        {
            ROS2::LidarRaycasterRequestBus::Handler::BusDisconnect();
        }

        if (m_rayPosesNode)
        {
            Utils::ErrorCheck(rgl_graph_destroy(m_pointsCompactNode));

            if (m_addMaxRangePoints)
            {
                Utils::ErrorCheck(rgl_graph_destroy(m_rayTraceNode));
            }
        }
    }

    void LidarRaycaster::ConfigureRayOrientations(const AZStd::vector<AZ::Vector3>& orientations)
    {
        ValidateRayOrientations(orientations);
        AZStd::vector<rgl_mat3x4f> rglRayTransforms;
        rglRayTransforms.reserve(orientations.size());
        for (const AZ::Vector3& orientation : orientations)
        {
            // Since we provide a transform for the Z axis unit vector we need an additional PI / 2 added to the pitch.
            const AZ::Matrix3x4 rayTransform = AZ::Matrix3x4::CreateFromQuaternion(AZ::Quaternion::CreateFromEulerRadiansZYX({
                orientation.GetX(),
                -orientation.GetY() + AZ::Constants::HalfPi,
                orientation.GetZ(),
            }));

            rglRayTransforms.push_back(Utils::RglMat3x4FromAzMatrix3x4(rayTransform));
        }

        m_rayTransforms.clear();
        m_rayTransforms.reserve(rglRayTransforms.size());
        for (rgl_mat3x4f transform : rglRayTransforms)
        {
            m_rayTransforms.push_back(Utils::AzMatrix3x4FromRglMat3x4(transform));
        }

        Utils::ErrorCheck(
            rgl_node_rays_from_mat3x4f(&m_rayPosesNode, rglRayTransforms.data(), aznumeric_cast<int32_t>(rglRayTransforms.size())));
    }

    void LidarRaycaster::ConfigureRayRange(float range)
    {
        ValidateRayRange(range);
        m_range = range;
        Utils::ErrorCheck(rgl_node_raytrace(&m_rayTraceNode, nullptr, range));
    }

    void LidarRaycaster::ConfigureMinimumRayRange(float range)
    {
        m_minRange = range;
    }

    void LidarRaycaster::ConfigureRaycastResultFlags(ROS2::RaycastResultFlags flags)
    {
        m_resultFlags = flags;
        m_resultFields = { RGL_FIELD_IS_HIT_I32 };

        if ((flags & ROS2::RaycastResultFlags::Points) == ROS2::RaycastResultFlags::Points)
        {
            m_resultFields.push_back(RGL_FIELD_XYZ_F32);
        }

        if ((flags & ROS2::RaycastResultFlags::Ranges) == ROS2::RaycastResultFlags::Ranges)
        {
            m_resultFields.push_back(RGL_FIELD_DISTANCE_F32);
        }

        Utils::ErrorCheck(
            rgl_node_points_format(&m_pointsFormatNode, m_resultFields.data(), aznumeric_cast<int32_t>(m_resultFields.size())));
        m_rglRaycastResults = RaycastResults{ m_resultFields, m_rglRaycastResults.GetCount() };
    }

    ROS2::RaycastResult LidarRaycaster::PerformRaycast(const AZ::Transform& lidarTransform)
    {
        const AZ::Matrix3x4 lidarPose = AZ::Matrix3x4::CreateFromTransform(lidarTransform);
        const rgl_mat3x4f rglLidarPose = Utils::RglMat3x4FromAzMatrix3x4(lidarPose);

        Utils::ErrorCheck(rgl_node_rays_transform(&m_lidarTransformNode, &rglLidarPose));
        Utils::ErrorCheck(rgl_graph_run(m_lidarTransformNode));

        int32_t resultSize = -1;
        Utils::ErrorCheck(rgl_graph_get_result_size(m_pointsFormatNode, rgl_field_t::RGL_FIELD_DYNAMIC_FORMAT, &resultSize, nullptr));

        if (resultSize <= 0)
        {
            return {};
        }

        m_rglRaycastResults.Resize(resultSize);
        Utils::ErrorCheck(
            rgl_graph_get_result_data(m_pointsFormatNode, rgl_field_t::RGL_FIELD_DYNAMIC_FORMAT, m_rglRaycastResults.GetData()));

        bool pointsExpected = (m_resultFlags & ROS2::RaycastResultFlags::Points) == ROS2::RaycastResultFlags::Points;
        bool distanceExpected = (m_resultFlags & ROS2::RaycastResultFlags::Ranges) == ROS2::RaycastResultFlags::Ranges;

        if (pointsExpected)
        {
            m_raycastResults.m_points.resize(resultSize);
        }

        if (distanceExpected)
        {
            m_raycastResults.m_ranges.resize(resultSize);
        }

        size_t usedIndex = 0LU;
        for (size_t resultIndex = 0LU; resultIndex < m_rglRaycastResults.GetCount(); ++resultIndex)
        {
            if (*static_cast<int32_t*>(m_rglRaycastResults.GetFieldPtr(resultIndex, RGL_FIELD_IS_HIT_I32)))
            {
                if (pointsExpected)
                {
                    m_raycastResults.m_points[usedIndex] = Utils::AzVector3FromRglVec3f(
                        *static_cast<rgl_vec3f*>(m_rglRaycastResults.GetFieldPtr(resultIndex, RGL_FIELD_XYZ_F32)));
                }

                if (distanceExpected)
                {
                    const float distance = *static_cast<float*>(m_rglRaycastResults.GetFieldPtr(resultIndex, RGL_FIELD_DISTANCE_F32));
                    if (distance > m_minRange)
                    {
                        m_raycastResults.m_ranges[usedIndex] = distance;
                    }
                }

                ++usedIndex;
            }
            else if (m_addMaxRangePoints)
            {
                if (pointsExpected)
                {
                    const AZ::Vector4 maxVector = lidarPose * m_rayTransforms[resultIndex] * AZ::Vector4(0.0f, 0.0f, m_range, 1.0f);
                    m_raycastResults.m_points[usedIndex] = maxVector.GetAsVector3();
                }

                if (distanceExpected)
                {
                    m_raycastResults.m_ranges[usedIndex] = m_range;
                }

                ++usedIndex;
            }
        }

        if (pointsExpected)
        {
            m_raycastResults.m_points.resize(usedIndex);
        }

        if (distanceExpected)
        {
            m_raycastResults.m_ranges.resize(usedIndex);
        }

        return m_raycastResults;
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
            Utils::ErrorCheck(rgl_graph_node_remove_child(m_rayTraceNode, m_pointsCompactNode));
            Utils::ErrorCheck(rgl_graph_node_remove_child(m_pointsCompactNode, m_pointsFormatNode));
            Utils::ErrorCheck(rgl_graph_node_add_child(m_rayTraceNode, m_pointsFormatNode));
        }
        else
        {
            Utils::ErrorCheck(rgl_graph_node_remove_child(m_rayTraceNode, m_pointsFormatNode));
            Utils::ErrorCheck(rgl_graph_node_add_child(m_rayTraceNode, m_pointsCompactNode));
            Utils::ErrorCheck(rgl_graph_node_add_child(m_pointsCompactNode, m_pointsFormatNode));
        }
    }
} // namespace RGL