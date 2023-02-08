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
    {
        ROS2::LidarRaycasterRequestBus::Handler::BusConnect(ROS2::LidarId(uuid));

        // Configure the default graph
        Utils::ErrorCheck(rgl_node_rays_from_mat3x4f(&m_rayPosesNode, &Utils::IdentityTransform, 1));
        Utils::ErrorCheck(rgl_node_rays_transform(&m_lidarTransformNode, &Utils::IdentityTransform));
        Utils::ErrorCheck(rgl_node_raytrace(&m_rayTraceNode, nullptr, m_range));
        Utils::ErrorCheck(rgl_node_points_compact(&m_pointsCompactNode));
        Utils::ErrorCheck(rgl_node_points_format(&m_pointsFormatNode, DefaultFields.data(), aznumeric_cast<int32_t>(DefaultFields.size())));

        Utils::ErrorCheck(rgl_graph_node_add_child(m_rayPosesNode, m_lidarTransformNode));
        Utils::ErrorCheck(rgl_graph_node_add_child(m_lidarTransformNode, m_rayTraceNode));
        Utils::ErrorCheck(rgl_graph_node_add_child(m_rayTraceNode, m_pointsCompactNode));
        Utils::ErrorCheck(rgl_graph_node_add_child(m_pointsCompactNode, m_pointsFormatNode));
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

    AZStd::vector<AZ::Vector3> LidarRaycaster::PerformRaycast(const AZ::Transform& lidarTransform)
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

        m_rglRaycastResults.resize(resultSize);
        Utils::ErrorCheck(rgl_graph_get_result_data(m_pointsFormatNode, rgl_field_t::RGL_FIELD_DYNAMIC_FORMAT, m_rglRaycastResults.data()));

        m_raycastResults.resize(resultSize);
        size_t usedIndex = 0LU;
        for (size_t resultIndex = 0LU; resultIndex < m_rglRaycastResults.size(); ++resultIndex)
        {
            if (m_rglRaycastResults[resultIndex].m_isHit)
            {
                m_raycastResults[usedIndex] = Utils::AzVector3FromRglVec3f(m_rglRaycastResults[resultIndex].m_xyz);

                ++usedIndex;
            }
            else if (m_addMaxRangePoints)
            {
                const AZ::Vector4 maxVector = lidarPose * m_rayTransforms[resultIndex] * AZ::Vector4(0.0f, 0.0f, m_range, 1.0f);
                m_raycastResults[usedIndex] = maxVector.GetAsVector3();

                ++usedIndex;
            }
        }

        m_raycastResults.resize(usedIndex);

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

    const AZStd::vector<rgl_field_t> LidarRaycaster::DefaultFields{ RGL_FIELD_XYZ_F32, RGL_FIELD_IS_HIT_I32 };
} // namespace RGL