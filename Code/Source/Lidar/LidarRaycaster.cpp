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
#include <ROS2/ROS2Bus.h>
#include <Utilities/RGLUtils.h>
#include <rgl/api/extensions/ros2.h>

namespace RGL
{
    LidarRaycaster::LidarRaycaster(const AZ::Uuid& uuid)
        : m_uuid{ uuid }
        , m_rglRaycastResults{ m_resultFields, 1LU }
        , m_graph{ m_range.second, m_resultFields }
    {
        ROS2::LidarRaycasterRequestBus::Handler::BusConnect(ROS2::LidarId(uuid));
    }

    LidarRaycaster::LidarRaycaster(LidarRaycaster&& other)
        : m_uuid{ other.m_uuid }
        , m_isMaxRangePoints{ other.m_isMaxRangePoints }
        , m_resultFlags{ other.m_resultFlags }
        , m_range{ other.m_range }
        , m_graph{ std::move(other.m_graph) }
        , m_resultFields{ AZStd::move(other.m_resultFields) }
        , m_rayTransforms{ AZStd::move(other.m_rayTransforms) }
        , m_rglRaycastResults{ AZStd::move(other.m_rglRaycastResults) }
    {
        other.BusDisconnect();

        // Ensure proper destruction of the movee.
        other.m_uuid = AZ::Uuid::CreateNull();
        ROS2::LidarRaycasterRequestBus::Handler::BusConnect(ROS2::LidarId(m_uuid));
    }

    LidarRaycaster::~LidarRaycaster()
    {
        if (!m_uuid.IsNull())
        {
            ROS2::LidarRaycasterRequestBus::Handler::BusDisconnect();
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

        RGL_CHECK(rgl_node_rays_from_mat3x4f(
            &m_graph.GetNodes().m_rayPoses, rglRayTransforms.data(), aznumeric_cast<int32_t>(rglRayTransforms.size())));
    }

    void LidarRaycaster::ConfigureRayRange(float range)
    {
        ValidateRayRange(range);
        m_range.second = range;
        RGL_CHECK(rgl_node_raytrace(&m_graph.GetNodes().m_rayTrace, nullptr, range));
    }

    void LidarRaycaster::ConfigureMinimumRayRange(float range)
    {
        m_range.first = range;
    }

    void LidarRaycaster::ConfigureRaycastResultFlags(ROS2::RaycastResultFlags flags)
    {
        m_resultFlags = flags;
        m_resultFields.clear();

        if ((flags & ROS2::RaycastResultFlags::Points) == ROS2::RaycastResultFlags::Points)
        {
            m_resultFields.push_back(RGL_FIELD_IS_HIT_I32);
            m_resultFields.push_back(RGL_FIELD_XYZ_F32);
        }

        if ((flags & ROS2::RaycastResultFlags::Ranges) == ROS2::RaycastResultFlags::Ranges)
        {
            m_resultFields.push_back(RGL_FIELD_DISTANCE_F32);
        }

        RGL_CHECK(rgl_node_points_format(
            &m_graph.GetNodes().m_pointsFormat, m_resultFields.data(), aznumeric_cast<int32_t>(m_resultFields.size())));
        RGL_CHECK(rgl_node_points_format(
            &m_graph.GetNodes().m_pointsFormatPublish, m_resultFields.data(), aznumeric_cast<int32_t>(m_resultFields.size())));

        m_rglRaycastResults = RaycastResults{ m_resultFields, m_rglRaycastResults.GetCount() };

        m_graph.SetIsCompactEnabled(ShouldEnableCompact());
        m_graph.SetIsPublishingEnabled(ShouldEnablePublishing());
    }

    ROS2::RaycastResult LidarRaycaster::PerformRaycast(const AZ::Transform& lidarTransform)
    {
        const AZ::Matrix3x4 lidarPose = AZ::Matrix3x4::CreateFromTransform(lidarTransform);
        m_lidarPose = Utils::RglMat3x4FromAzMatrix3x4(lidarPose);
        m_lidarPoseInv = Utils::RglMat3x4FromAzMatrix3x4(lidarPose.GetInverseFull());

        RGL_CHECK(rgl_node_rays_transform(&m_graph.GetNodes().m_lidarTransform, &m_lidarPose));
        if (m_graph.IsPublishingEnabled())
        {
            RGL_CHECK(rgl_node_points_transform(&m_graph.GetNodes().m_pointCloudTransform, &m_lidarPoseInv));
        }

        RGL_CHECK(rgl_graph_run(m_graph.GetNodes().m_lidarTransform));

        int32_t resultSize = -1;
        RGL_CHECK(
            rgl_graph_get_result_size(m_graph.GetNodes().m_pointsFormat, rgl_field_t::RGL_FIELD_DYNAMIC_FORMAT, &resultSize, nullptr));

        if (resultSize <= 0)
        {
            return {};
        }

        m_rglRaycastResults.Resize(resultSize);
        RGL_CHECK(rgl_graph_get_result_data(
            m_graph.GetNodes().m_pointsFormat, rgl_field_t::RGL_FIELD_DYNAMIC_FORMAT, m_rglRaycastResults.GetData()));

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

        size_t usedPointIndex = 0LU;
        const float MaxRange = m_isMaxRangePoints ? m_range.second : AZStd::numeric_limits<float>::infinity();
        for (size_t resultIndex = 0LU; resultIndex < m_rglRaycastResults.GetCount(); ++resultIndex)
        {
            if (pointsExpected)
            {
                const bool IsHit = (m_graph.IsCompactEnabled()) ||
                    *static_cast<int32_t*>(m_rglRaycastResults.GetFieldPtr(resultIndex, RGL_FIELD_IS_HIT_I32));
                if (IsHit)
                {
                    m_raycastResults.m_points[usedPointIndex] = Utils::AzVector3FromRglVec3f(
                        *static_cast<rgl_vec3f*>(m_rglRaycastResults.GetFieldPtr(resultIndex, RGL_FIELD_XYZ_F32)));
                }
                else if (m_isMaxRangePoints)
                {
                    const AZ::Vector4 maxVector = lidarPose * m_rayTransforms[resultIndex] * AZ::Vector4(0.0f, 0.0f, m_range.second, 1.0f);
                    m_raycastResults.m_points[usedPointIndex] = maxVector.GetAsVector3();
                }

                if (IsHit || m_isMaxRangePoints)
                {
                    ++usedPointIndex;
                }
            }

            if (distanceExpected)
            {
                float distance = *static_cast<float*>(m_rglRaycastResults.GetFieldPtr(resultIndex, RGL_FIELD_DISTANCE_F32));
                if (distance < m_range.first)
                {
                    distance = -AZStd::numeric_limits<float>::infinity();
                }
                else if (distance > m_range.second)
                {
                    distance = MaxRange;
                }

                m_raycastResults.m_ranges[resultIndex] = distance;
            }
        }

        if (pointsExpected)
        {
            m_raycastResults.m_points.resize(usedPointIndex);
        }

        return m_raycastResults;
    }

    void LidarRaycaster::ConfigureNoiseParameters(
        float angularNoiseStdDev, float distanceNoiseStdDevBase, float distanceNoiseStdDevRisePerMeter)
    {
        RGL_CHECK(rgl_node_gaussian_noise_angular_ray(&m_graph.GetNodes().m_angularNoise, 0.0f, angularNoiseStdDev, RGL_AXIS_Z));
        RGL_CHECK(rgl_node_gaussian_noise_distance(
            &m_graph.GetNodes().m_distanceNoise, 0.0f, distanceNoiseStdDevBase, distanceNoiseStdDevRisePerMeter));

        m_graph.SetIsNoiseEnabled(true);
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
        m_isMaxRangePoints = addMaxRangePoints;

        // We need to configure if points should be compacted to minimize the CPU operations when retrieving raycast results.
        m_graph.SetIsCompactEnabled(ShouldEnableCompact());

        m_graph.SetIsPublishingEnabled(ShouldEnablePublishing());
    }

    void LidarRaycaster::ConfigurePointCloudPublisher(
        const AZStd::string& topicName, const AZStd::string& frameId, const ROS2::QoS& qoSPolicy)
    {
        RGL_CHECK(rgl_node_points_transform(&m_graph.GetNodes().m_pointCloudTransform, &Utils::IdentityTransform));

        RGL_CHECK(rgl_node_points_ros2_publish_with_qos(
            &m_graph.GetNodes().m_pointCloudPublish,
            topicName.c_str(),
            frameId.c_str(),
            static_cast<rgl_qos_policy_reliability_t>(static_cast<int>(qoSPolicy.GetQoS().reliability())),
            static_cast<rgl_qos_policy_durability_t>(static_cast<int>(qoSPolicy.GetQoS().durability())),
            static_cast<rgl_qos_policy_history_t>(static_cast<int>(qoSPolicy.GetQoS().history())),
            qoSPolicy.GetQoS().depth()));

        m_graph.SetIsPublishingEnabled(ShouldEnablePublishing());
    }

    bool LidarRaycaster::CanHandlePublishing()
    {
        return m_graph.IsPublishingEnabled();
    }

    void LidarRaycaster::UpdatePublisherTimestamp(AZ::u64 timestampNanoseconds)
    {
        RGL_CHECK(rgl_scene_set_time(nullptr, timestampNanoseconds));
    }

    bool LidarRaycaster::ShouldEnableCompact() const
    {
        const bool RangesExpected = (m_resultFlags & ROS2::RaycastResultFlags::Ranges) == ROS2::RaycastResultFlags::Ranges;
        return !RangesExpected && !m_isMaxRangePoints;
    }

    bool LidarRaycaster::ShouldEnablePublishing() const
    {
        // Same conditions.
        return m_graph.IsPublisherConfigured() && ShouldEnableCompact();
    }
} // namespace RGL
