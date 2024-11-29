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
#include <Lidar/LidarSystemNotificationBus.h>
#include <RGL/RGLBus.h>
#include <ROS2/ROS2Bus.h>
#include <Utilities/RGLUtils.h>

namespace RGL
{
    LidarRaycaster::LidarRaycaster(const AZ::Uuid& uuid)
        : m_uuid{ uuid }
    {
        ROS2::LidarRaycasterRequestBus::Handler::BusConnect(ROS2::LidarId(uuid));
        LidarSystemNotificationBus::Broadcast(&LidarSystemNotifications::OnLidarCreated);
    }

    LidarRaycaster::LidarRaycaster(LidarRaycaster&& other)
        : m_uuid{ other.m_uuid }
        , m_returnNonHits{ other.m_returnNonHits }
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
            LidarSystemNotificationBus::Broadcast(&LidarSystemNotifications::OnLidarDestroyed);
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

    void LidarRaycaster::ConfigureRayRange(ROS2::RayRange range)
    {
        m_range = range;

        UpdateNonHitValues();

        m_graph.ConfigureRayRangesNode(range.m_min, range.m_max);
    }

    void LidarRaycaster::ConfigureRaycastResultFlags(ROS2::RaycastResultFlags flags)
    {
        m_rglRaycastResults = {};
        m_raycastResults = ROS2::RaycastResults(flags);

        m_rglRaycastResults.m_fields = { RGL_FIELD_IS_HIT_I32 };
        if (ROS2::IsFlagEnabled(ROS2::RaycastResultFlags::Point, flags))
        {
            m_rglRaycastResults.m_fields.push_back(RGL_FIELD_XYZ_VEC3_F32);
        }

        if (ROS2::IsFlagEnabled(ROS2::RaycastResultFlags::Range, flags))
        {
            m_rglRaycastResults.m_fields.push_back(RGL_FIELD_DISTANCE_F32);
        }

        if (ROS2::IsFlagEnabled(ROS2::RaycastResultFlags::Intensity, flags))
        {
            m_rglRaycastResults.m_fields.push_back(RGL_FIELD_INTENSITY_F32);
        }

        if (ROS2::IsFlagEnabled(ROS2::RaycastResultFlags::SegmentationData, flags))
        {
            m_rglRaycastResults.m_fields.push_back(RGL_FIELD_ENTITY_ID_I32);
        }

        if (ROS2::IsFlagEnabled(ROS2::RaycastResultFlags::Ring, flags))
        {
            m_rglRaycastResults.m_fields.push_back(RGL_FIELD_RING_ID_U16);
        }

        m_graph.ConfigureFieldNodes(m_rglRaycastResults.m_fields.data(), m_rglRaycastResults.m_fields.size());
        m_graph.SetIsCompactEnabled(!m_returnNonHits);
    }

    AZ::Outcome<ROS2::RaycastResults, const char*> LidarRaycaster::PerformRaycast(const AZ::Transform& lidarTransform)
    {
        AZ_Assert(m_range.has_value(), "Programmer error. Raycaster range is not fully configured.");
        AZ_Assert(m_raycastResults.has_value(), "Programmer error. Raycaster result fields not fully configured.");
        RGLInterface::Get()->UpdateScene();

        const AZ::Matrix3x4 lidarPose = AZ::Matrix3x4::CreateFromTransform(lidarTransform);

        m_graph.ConfigureLidarTransformNode(lidarPose);

        m_graph.Run();

        if (!m_graph.GetResults(m_rglRaycastResults))
        {
            return AZ::Failure("Results returned by RGL did not match requested.");
        }

        const auto resultSize = GetRglResultsSize(m_rglRaycastResults, m_raycastResults.value());
        if (!resultSize.has_value())
        {
            return AZ::Failure("Results were of different sizes.");
        }

        ROS2::RaycastResults& raycastResults = m_raycastResults.value();
        raycastResults.Resize(resultSize.value());

        if (auto points = raycastResults.GetFieldSpan<ROS2::RaycastResultFlags::Point>(); points.has_value())
        {
            AZStd::transform(
                m_rglRaycastResults.m_xyz.begin(), m_rglRaycastResults.m_xyz.end(), points.value().begin(), Utils::AzVector3FromRglVec3f);
        }

        if (auto distance = raycastResults.GetFieldSpan<ROS2::RaycastResultFlags::Range>(); distance.has_value())
        {
            AZStd::copy(m_rglRaycastResults.m_distance.begin(), m_rglRaycastResults.m_distance.end(), distance.value().begin());
        }

        if (auto intensity = raycastResults.GetFieldSpan<ROS2::RaycastResultFlags::Intensity>(); intensity.has_value())
        {
            AZStd::copy(m_rglRaycastResults.m_intensity.begin(), m_rglRaycastResults.m_intensity.end(), intensity.value().begin());
        }

        if (auto segmentationData = raycastResults.GetFieldSpan<ROS2::RaycastResultFlags::SegmentationData>(); segmentationData.has_value())
        {
            AZStd::transform(
                m_rglRaycastResults.m_packedRglEntityId.begin(),
                m_rglRaycastResults.m_packedRglEntityId.end(),
                segmentationData.value().begin(),
                Utils::UnpackRglEntityId);
        }

        if (auto isHit = raycastResults.GetFieldSpan<ROS2::RaycastResultFlags::IsHit>(); isHit.has_value())
        {
            AZStd::transform(
                m_rglRaycastResults.m_isHit.begin(),
                m_rglRaycastResults.m_isHit.end(),
                isHit.value().begin(),
                [](int32_t isHit)
                {
                    return isHit != 0;
                });
        }

        if (auto ring = raycastResults.GetFieldSpan<ROS2::RaycastResultFlags::Ring>(); ring.has_value())
        {
            AZStd::copy(m_rglRaycastResults.m_ringId.begin(), m_rglRaycastResults.m_ringId.end(), ring.value().begin());
        }

        return AZ::Success(m_raycastResults.value());
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

    void LidarRaycaster::UpdateNonHitValues()
    {
        float minRangeNonHitValue = -AZStd::numeric_limits<float>::infinity();
        float maxRangeNonHitValue = AZStd::numeric_limits<float>::infinity();

        if (m_returnNonHits && m_range.has_value())
        {
            minRangeNonHitValue = m_range.value().m_min;
            maxRangeNonHitValue = m_range.value().m_max;
        }

        m_graph.ConfigureRaytraceNodeNonHits(minRangeNonHitValue, maxRangeNonHitValue);
    }

    void LidarRaycaster::ConfigureNonHitReturn(bool returnNonHits)
    {
        m_returnNonHits = returnNonHits;
        UpdateNonHitValues();

        // We need to configure if points should be compacted to minimize the CPU operations when retrieving raycast results.
        m_graph.SetIsCompactEnabled(!returnNonHits);
    }

    void LidarRaycaster::ConfigureRayRingIds(const AZStd::vector<AZ::s32>& ringIds)
    {
        m_graph.ConfigureRayRingIds(ringIds);
    }

    AZStd::optional<size_t> LidarRaycaster::GetRglResultsSize(
        const PipelineGraph::RaycastResults& rglResults, const ROS2::RaycastResults& results)
    {
        // Size consistent among all (present) fields.
        // If no consensus is reached or no fields are present, set to AZStd::nullopt.
        AZStd::optional<size_t> resultsSize;
        if (results.IsFieldPresent<ROS2::RaycastResultFlags::Point>())
        {
            resultsSize = rglResults.m_xyz.size();
        }

        if (results.IsFieldPresent<ROS2::RaycastResultFlags::Range>())
        {
            if (!resultsSize.has_value())
            {
                resultsSize = rglResults.m_distance.size();
            }
            else if (resultsSize != rglResults.m_distance.size())
            {
                return AZStd::nullopt;
            }
        }

        if (results.IsFieldPresent<ROS2::RaycastResultFlags::Intensity>())
        {
            if (!resultsSize.has_value())
            {
                resultsSize = rglResults.m_intensity.size();
            }
            else if (resultsSize != rglResults.m_intensity.size())
            {
                return AZStd::nullopt;
            }
        }

        if (results.IsFieldPresent<ROS2::RaycastResultFlags::SegmentationData>())
        {
            if (!resultsSize.has_value())
            {
                resultsSize = rglResults.m_packedRglEntityId.size();
            }
            else if (resultsSize != rglResults.m_packedRglEntityId.size())
            {
                return AZStd::nullopt;
            }
        }

        if (results.IsFieldPresent<ROS2::RaycastResultFlags::IsHit>())
        {
            if (!resultsSize.has_value())
            {
                resultsSize = rglResults.m_isHit.size();
            }
            else if (resultsSize != rglResults.m_isHit.size())
            {
                return AZStd::nullopt;
            }
        }

        if (results.IsFieldPresent<ROS2::RaycastResultFlags::Ring>())
        {
            if (!resultsSize.has_value())
            {
                resultsSize = rglResults.m_ringId.size();
            }
            else if (resultsSize != rglResults.m_ringId.size())
            {
                return AZStd::nullopt;
            }
        }

        return resultsSize;
    }
} // namespace RGL
