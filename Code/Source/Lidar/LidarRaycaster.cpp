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
    {
        ROS2::LidarRaycasterRequestBus::Handler::BusConnect(ROS2::LidarId(uuid));
    }

    LidarRaycaster::LidarRaycaster(LidarRaycaster&& other)
        : m_uuid{ other.m_uuid }
        , m_isMaxRangeEnabled{ other.m_isMaxRangeEnabled }
        , m_resultFlags{ other.m_resultFlags }
        , m_range{ other.m_range }
        , m_graph{ std::move(other.m_graph) }
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

        m_graph.ConfigureRayPosesNode(rglRayTransforms);
    }

    void LidarRaycaster::ConfigureRayRange(float range)
    {
        ValidateRayRange(range);
        m_range.second = range;
        // We set the graph-side value of min range to zero to be able to distinguish rays below min range from the ones above max range.
        m_graph.ConfigureRayRangesNode(0.0f, m_range.second);
    }

    void LidarRaycaster::ConfigureMinimumRayRange(float range)
    {
        m_range.first = range;
        // We omit updating the graph-side value of min range to be able to distinguish rays below min range from the ones above max range.
    }

    void LidarRaycaster::ConfigureRaycastResultFlags(ROS2::RaycastResultFlags flags)
    {
        m_resultFlags = flags;
        m_rglRaycastResults.m_fields.clear();
        m_rglRaycastResults.m_isHit.clear();
        m_rglRaycastResults.m_xyz.clear();
        m_rglRaycastResults.m_distance.clear();

        if ((flags & ROS2::RaycastResultFlags::Points) == ROS2::RaycastResultFlags::Points)
        {
            m_rglRaycastResults.m_fields.push_back(RGL_FIELD_IS_HIT_I32);
            m_rglRaycastResults.m_fields.push_back(RGL_FIELD_XYZ_F32);
            m_rglRaycastResults.m_fields.push_back(RGL_FIELD_ENTITY_ID_I32);
        }

        if ((flags & ROS2::RaycastResultFlags::Ranges) == ROS2::RaycastResultFlags::Ranges)
        {
            m_rglRaycastResults.m_fields.push_back(RGL_FIELD_DISTANCE_F32);
        }

        m_graph.ConfigureYieldNodes(m_rglRaycastResults.m_fields.data(), m_rglRaycastResults.m_fields.size());

        m_graph.SetIsCompactEnabled(ShouldEnableCompact());
        m_graph.SetIsPcPublishingEnabled(ShouldEnablePcPublishing());
    }

    ROS2::RaycastResult LidarRaycaster::PerformRaycast(const AZ::Transform& lidarTransform)
    {
        RGLInterface::Get()->UpdateScene();

        const AZ::Matrix3x4 lidarPose = AZ::Matrix3x4::CreateFromTransform(lidarTransform);

        m_graph.ConfigureLidarTransformNode(lidarPose);
        if (m_graph.IsPcPublishingEnabled())
        {
            // Transforms the obtained point-cloud from world to sensor frame of reference.
            m_graph.ConfigurePcTransformNode(lidarPose.GetInverseFull());
        }

        m_graph.Run();

        if (!m_graph.GetResults(m_rglRaycastResults))
        {
            return {};
        }

        bool pointsExpected = (m_resultFlags & ROS2::RaycastResultFlags::Points) == ROS2::RaycastResultFlags::Points;
        bool distanceExpected = (m_resultFlags & ROS2::RaycastResultFlags::Ranges) == ROS2::RaycastResultFlags::Ranges;

        if (pointsExpected)
        {
            m_raycastResults.m_points.resize(m_rglRaycastResults.m_xyz.size());
            m_raycastResults.m_ids.resize(m_raycastResults.m_points.size()); // TODO: Decide on desired default value here.
        }

        if (distanceExpected)
        {
            m_raycastResults.m_ranges.resize(m_rglRaycastResults.m_distance.size());
        }

        size_t usedPointIndex = 0LU;
        const size_t resultsSize = pointsExpected ? m_raycastResults.m_points.size() : m_raycastResults.m_ranges.size();
        const float maxRange = m_isMaxRangeEnabled ? m_range.second : AZStd::numeric_limits<float>::infinity();
        for (size_t resultIndex = 0LU; resultIndex < resultsSize; ++resultIndex)
        {
            if (pointsExpected)
            {
                const bool isHit = (m_graph.IsCompactEnabled()) || aznumeric_cast<bool>(m_rglRaycastResults.m_isHit[resultIndex]);
                if (isHit)
                {
                    m_raycastResults.m_points[usedPointIndex] = Utils::AzVector3FromRglVec3f(m_rglRaycastResults.m_xyz[resultIndex]);
                }
                else if (m_isMaxRangeEnabled)
                {
                    const AZ::Vector4 maxVector = lidarPose * m_rayTransforms[resultIndex] * AZ::Vector4(0.0f, 0.0f, m_range.second, 1.0f);
                    m_raycastResults.m_points[usedPointIndex] = maxVector.GetAsVector3();
                }

                if (isHit || m_isMaxRangeEnabled)
                {
                    m_raycastResults.m_ids[usedPointIndex] = m_rglRaycastResults.m_entityId[resultIndex];
                    ++usedPointIndex;
                }
            }

            if (distanceExpected)
            {
                float distance = m_rglRaycastResults.m_distance[resultIndex];
                if (distance < m_range.first)
                {
                    distance = -AZStd::numeric_limits<float>::infinity();
                }
                else if (distance > m_range.second)
                {
                    distance = maxRange;
                }

                m_raycastResults.m_ranges[resultIndex] = distance;
            }
        }

        if (pointsExpected)
        {
            m_raycastResults.m_points.resize(usedPointIndex);
            m_raycastResults.m_ids.resize(usedPointIndex);
        }

        return m_raycastResults;
    }

    void LidarRaycaster::ConfigureNoiseParameters(
        float angularNoiseStdDev, float distanceNoiseStdDevBase, float distanceNoiseStdDevRisePerMeter)
    {
        m_graph.ConfigureAngularNoiseNode(angularNoiseStdDev);
        m_graph.ConfigureDistanceNoiseNode(distanceNoiseStdDevBase, distanceNoiseStdDevRisePerMeter);
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
        m_isMaxRangeEnabled = addMaxRangePoints;

        // We need to configure if points should be compacted to minimize the CPU operations when retrieving raycast results.
        m_graph.SetIsCompactEnabled(ShouldEnableCompact());
        m_graph.SetIsPcPublishingEnabled(ShouldEnablePcPublishing());
    }

    void LidarRaycaster::ConfigurePointCloudPublisher(
        const AZStd::string& topicName, const AZStd::string& frameId, const ROS2::QoS& qosPolicy)
    {
        m_graph.ConfigurePcPublisherNode(topicName, frameId, qosPolicy);
        m_graph.SetIsPcPublishingEnabled(ShouldEnablePcPublishing());
    }

    bool LidarRaycaster::CanHandlePublishing()
    {
        return m_graph.IsPcPublishingEnabled();
    }

    void LidarRaycaster::UpdatePublisherTimestamp(AZ::u64 timestampNanoseconds)
    {
        RGL_CHECK(rgl_scene_set_time(nullptr, timestampNanoseconds));
    }

    bool LidarRaycaster::ArePointsExpected() const
    {
        return (m_resultFlags & ROS2::RaycastResultFlags::Points) == ROS2::RaycastResultFlags::Points;
    }
    bool LidarRaycaster::AreRangesExpected() const
    {
        return (m_resultFlags & ROS2::RaycastResultFlags::Ranges) == ROS2::RaycastResultFlags::Ranges;
    }

    bool LidarRaycaster::ShouldEnableCompact() const
    {
        return !AreRangesExpected() && !m_isMaxRangeEnabled;
    }

    bool LidarRaycaster::ShouldEnablePcPublishing() const
    {
        return m_graph.IsPublisherConfigured() && !AreRangesExpected() && !m_isMaxRangeEnabled;
    }
} // namespace RGL
