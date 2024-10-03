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
#pragma once

#include <Lidar/PipelineGraph.h>
#include <ROS2/Lidar/LidarRaycasterBus.h>
#include <Utilities/RGLUtils.h>
#include <rgl/api/core.h>

namespace RGL
{
    class LidarRaycaster : protected ROS2::LidarRaycasterRequestBus::Handler
    {
    public:
        explicit LidarRaycaster(const AZ::Uuid& uuid);
        LidarRaycaster(LidarRaycaster&& other);
        LidarRaycaster(const LidarRaycaster& other) = delete;
        ~LidarRaycaster() override;

    protected:
        // LidarRaycasterRequestBus overrides
        void ConfigureRayOrientations(const AZStd::vector<AZ::Vector3>& orientations) override;
        void ConfigureRayRange(ROS2::RayRange range) override;
        void ConfigureRaycastResultFlags(ROS2::RaycastResultFlags flags) override;
        bool CanHandlePublishing() override;

        // Returns the size of required results.
        // If obtained result sizes do not match,
        // the returned optional does not contain a value.
        static AZStd::optional<size_t> GetRglResultsSize(
            const PipelineGraph::RaycastResults& rglResults, const ROS2::RaycastResults& results);

        AZ::Outcome<ROS2::RaycastResults, const char*> PerformRaycast(const AZ::Transform& lidarTransform) override;

        void ConfigureNoiseParameters(
            [[maybe_unused]] float angularNoiseStdDev,
            [[maybe_unused]] float distanceNoiseStdDevBase,
            [[maybe_unused]] float distanceNoiseStdDevRisePerMeter) override;
        void ExcludeEntities(const AZStd::vector<AZ::EntityId>& excludedEntities) override;
        void UpdateNonHitValues();
        void ConfigureMaxRangePointAddition(bool addMaxRangePoints) override;

        void ConfigurePointCloudPublisher(
            [[maybe_unused]] const AZStd::string& topicName,
            [[maybe_unused]] const AZStd::string& frameId,
            [[maybe_unused]] const ROS2::QoS& qosPolicy) override;

        void UpdatePublisherTimestamp([[maybe_unused]] AZ::u64 timestampNanoseconds) override;

    private:
        AZ::Uuid m_uuid;

        bool m_isMaxRangeEnabled{ false }; //!< Determines whether max range point addition is enabled.

        AZStd::optional<ROS2::RayRange> m_range{};
        AZStd::vector<AZ::Matrix3x4> m_rayTransforms{ AZ::Matrix3x4::CreateIdentity() };

        PipelineGraph::RaycastResults m_rglRaycastResults;
        AZStd::optional<ROS2::RaycastResults> m_raycastResults;

        PipelineGraph m_graph;

        [[nodiscard]] bool ShouldEnableCompact() const;
    };
} // namespace RGL
