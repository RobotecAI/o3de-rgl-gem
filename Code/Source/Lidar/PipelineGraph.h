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

#include <AzCore/Math/Matrix3x3.h>
#include <AzCore/std/containers/array.h>
#include <ROS2/Communication/QoS.h>
#include <rgl/api/core.h>
#include <Utilities/RGLUtils.h>

namespace RGL
{
    //! Class that manages the RGL pipeline graph construction, which depends on
    //! three conditions: point-cloud compact, noise and publication. The diagram
    //! representation of this graph can be found under static/PipelineGraph.mmd.
    class PipelineGraph
    {
    private:
        static constexpr AZStd::array<rgl_field_t, 2> DefaultFields{ RGL_FIELD_IS_HIT_I32, RGL_FIELD_XYZ_F32 };

    public:
        struct RaycastResults
        {
            AZStd::vector<rgl_field_t> m_fields{ DefaultFields.data(), DefaultFields.data() + DefaultFields.size() };
            AZStd::vector<int32_t> m_isHit;
            AZStd::vector<rgl_vec3f> m_xyz;
            AZStd::vector<float> m_distance;
        };

        struct Nodes
        {
            rgl_node_t m_rayPoses{ nullptr }, m_rayRanges{ nullptr }, m_lidarTransform{ nullptr }, m_angularNoise{ nullptr },
                m_rayTrace{ nullptr }, m_distanceNoise{ nullptr }, m_rayTraceYield{ nullptr }, m_pointsCompact{ nullptr },
                m_compactYield{ nullptr }, m_pointsYield{ nullptr }, m_pointCloudTransform{ nullptr }, m_pcPublishFormat{ nullptr },
                m_pointCloudPublish{ nullptr };
        };

        PipelineGraph();
        PipelineGraph(const PipelineGraph& other) = delete;
        PipelineGraph(PipelineGraph&& other);
        ~PipelineGraph();

        [[nodiscard]] bool IsCompactEnabled() const;
        [[nodiscard]] bool IsPcPublishingEnabled() const;
        [[nodiscard]] bool IsNoiseEnabled() const;
        [[nodiscard]] bool IsPublisherConfigured() const
        {
            return m_nodes.m_pointCloudPublish;
        }

        void ConfigureRayPosesNode(const AZStd::vector<rgl_mat3x4f>& rayPoses);
        void ConfigureRayRangesNode(float minRange, float maxRange);
        void ConfigureYieldNodes(const rgl_field_t* fields, size_t size);
        void ConfigureLidarTransformNode(const AZ::Matrix3x4& lidarTransform);
        void ConfigurePcTransformNode(const AZ::Matrix3x4& pcTransform);
        void ConfigureAngularNoiseNode(float angularNoiseStdDev);
        void ConfigureDistanceNoiseNode(float distanceNoiseStdDevBase, float distanceNoiseStdDevRisePerMeter);
        void ConfigurePcPublisherNode(const AZStd::string& topicName, const AZStd::string& frameId, const ROS2::QoS& qosPolicy);

        void SetIsCompactEnabled(bool value);
        void SetIsPcPublishingEnabled(bool value);
        void SetIsNoiseEnabled(bool value);

        void Run();

        //! Get the raycast results.
        //! @param results Raycast results destination.
        //! @return If successful returns true, otherwise returns false.
        bool GetResults(RaycastResults& results) const;

    private:
        enum PipelineFeatureFlags : uint8_t
        // clang-format off
        {
            None                    = 0,
            Noise                   = 1,
            PointsCompact           = 1 << 1,
            PointCloudPublishing    = 1 << 2,
            All                     = Noise | PointsCompact | PointCloudPublishing,
        };
        // clang-format on

        using ConditionType = AZStd::function<bool(const PipelineGraph&)>;

        class ConditionalConnection
        {
        public:
            ConditionalConnection(rgl_node_t parent, rgl_node_t child, const ConditionType& condition, bool activate = false);
            void Update(const PipelineGraph& graph);

        private:
            bool m_isActive;
            ConditionType m_condition;
            rgl_node_t m_parent, m_child;
        };

        [[nodiscard]] bool IsFeatureEnabled(PipelineFeatureFlags feature) const;

        //! Get a raycast result of specified field.
        //! @param result Raycast field result vector.
        //! @param rglFieldType Enum value representing the field type.
        //! @return If successful returns true, otherwise returns false.
        template<typename FieldType>
        bool GetResult(AZStd::vector<FieldType>& result, rgl_field_t rglFieldType) const
        {
            int32_t resultSize = -1;
            RGL_CHECK(rgl_graph_get_result_size(m_nodes.m_pointsYield, rglFieldType, &resultSize, nullptr));

            if (resultSize <= 0)
            {
                return false;
            }

            result.resize(resultSize);
            bool success = false;
            Utils::ErrorCheck(rgl_graph_get_result_data(m_nodes.m_pointsYield, rglFieldType, result.data()), __FILE__, __LINE__, &success);
            return success;
        }

        void SetIsFeatureEnabled(PipelineFeatureFlags feature, bool value);
        void InitializeConditionalConnections();
        void UpdateConnections();
        //! Adds conditional connections that support conditional insertion / removal
        //! of a node from in between the parent and child node.
        //! @param condition When true, the node should connect with parent and child.
        //! Otherwise the node is not connected.
        void AddConditionalNode(rgl_node_t node, rgl_node_t parent, rgl_node_t child, const ConditionType& condition);
        void AddConditionalConnection(rgl_node_t parent, rgl_node_t child, const ConditionType& condition);

        PipelineFeatureFlags m_activeFeatures{ PointsCompact };
        Nodes m_nodes;
        std::vector<ConditionalConnection> m_conditionalConnections;
    };
} // namespace RGL
