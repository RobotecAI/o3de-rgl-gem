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
#include <Lidar/PipelineGraph.h>
#include <Utilities/RGLUtils.h>
#include <rgl/api/extensions/ros2.h>

namespace RGL
{
    PipelineGraph::PipelineGraph()
    {
        ConfigureRayPosesNode({ Utils::IdentityTransform });
        ConfigureRayRangesNode(0.0f, 1.0f);
        ConfigureLidarTransformNode(AZ::Matrix3x4::CreateIdentity());
        RGL_CHECK(rgl_node_raytrace(&m_nodes.m_rayTrace, nullptr));
        RGL_CHECK(rgl_node_points_compact_by_field(&m_nodes.m_pointsCompact, RGL_FIELD_IS_HIT_I32));
        ConfigureAngularNoiseNode(0.0f);
        ConfigureDistanceNoiseNode(0.0f, 0.0f);
        ConfigureYieldNodes(DefaultFields.data(), DefaultFields.size());

        ConfigurePcTransformNode(AZ::Matrix3x4::CreateIdentity());
        RGL_CHECK(rgl_node_points_format(&m_nodes.m_pcPublishFormat, DefaultFields.data(), aznumeric_cast<int32_t>(DefaultFields.size())));

        // Non-conditional connections
        RGL_CHECK(rgl_graph_node_add_child(m_nodes.m_rayPoses, m_nodes.m_rayRanges));
        RGL_CHECK(rgl_graph_node_add_child(m_nodes.m_rayRanges, m_nodes.m_lidarTransform));
        RGL_CHECK(rgl_graph_node_add_child(m_nodes.m_compactYield, m_nodes.m_pointsYield));
        RGL_CHECK(rgl_graph_node_add_child(m_nodes.m_pointCloudTransform, m_nodes.m_pcPublishFormat));

        InitializeConditionalConnections();
    }

    PipelineGraph::PipelineGraph(PipelineGraph&& other)
        : m_nodes{ other.m_nodes }
        , m_activeFeatures{ other.m_activeFeatures }
        , m_conditionalConnections(std::move(other.m_conditionalConnections))
    {
        other.m_nodes = {};
        other.m_conditionalConnections.clear();
    }

    PipelineGraph::~PipelineGraph()
    {
        if (!m_nodes.m_rayPoses)
        {
            return;
        }

        // We enable all the features we can to destroy the whole graph with
        // one (or two) rgl_graph_destroy API call(s).
        SetIsNoiseEnabled(true);
        SetIsCompactEnabled(true);
        if (IsPublisherConfigured())
        {
            SetIsPcPublishingEnabled(true);
        }
        else
        {
            rgl_graph_destroy(m_nodes.m_pointCloudTransform);
        }

        rgl_graph_destroy(m_nodes.m_rayPoses);
    }

    bool PipelineGraph::IsCompactEnabled() const
    {
        return IsFeatureEnabled(PipelineFeatureFlags::PointsCompact);
    }
    bool PipelineGraph::IsPcPublishingEnabled() const
    {
        return IsFeatureEnabled(PipelineFeatureFlags::PointCloudPublishing);
    }
    bool PipelineGraph::IsNoiseEnabled() const
    {
        return IsFeatureEnabled(PipelineFeatureFlags::Noise);
    }

    void PipelineGraph::ConfigureRayPosesNode(const AZStd::vector<rgl_mat3x4f>& rayPoses)
    {
        RGL_CHECK(rgl_node_rays_from_mat3x4f(&m_nodes.m_rayPoses, rayPoses.data(), aznumeric_cast<int32_t>(rayPoses.size())));
    }

    void PipelineGraph::ConfigureRayRangesNode(float min, float max)
    {
        const rgl_vec2f range = { .value = { min, max } };
        RGL_CHECK(rgl_node_rays_set_range(&m_nodes.m_rayRanges, &range, 1));
    }

    void PipelineGraph::ConfigureYieldNodes(const rgl_field_t* fields, size_t size)
    {
        RGL_CHECK(rgl_node_points_yield(&m_nodes.m_pointsYield, fields, aznumeric_cast<int32_t>(size)));
        RGL_CHECK(rgl_node_points_yield(&m_nodes.m_rayTraceYield, fields, aznumeric_cast<int32_t>(size)));
        RGL_CHECK(rgl_node_points_yield(&m_nodes.m_compactYield, fields, aznumeric_cast<int32_t>(size)));
    }

    void PipelineGraph::ConfigureLidarTransformNode(const AZ::Matrix3x4& lidarTransform)
    {
        const rgl_mat3x4f RglLidarTransform = Utils::RglMat3x4FromAzMatrix3x4(lidarTransform);
        RGL_CHECK(rgl_node_rays_transform(&m_nodes.m_lidarTransform, &RglLidarTransform));
    }

    void PipelineGraph::ConfigurePcTransformNode(const AZ::Matrix3x4& pcTransform)
    {
        const rgl_mat3x4f rglPcTransform = Utils::RglMat3x4FromAzMatrix3x4(pcTransform);
        RGL_CHECK(rgl_node_points_transform(&m_nodes.m_pointCloudTransform, &rglPcTransform));
    }

    void PipelineGraph::ConfigureAngularNoiseNode(float angularNoiseStdDev)
    {
        RGL_CHECK(rgl_node_gaussian_noise_angular_ray(&m_nodes.m_angularNoise, 0.0f, angularNoiseStdDev, RGL_AXIS_Z));
    }

    void PipelineGraph::ConfigureDistanceNoiseNode(float distanceNoiseStdDevBase, float distanceNoiseStdDevRisePerMeter)
    {
        RGL_CHECK(
            rgl_node_gaussian_noise_distance(&m_nodes.m_distanceNoise, 0.0f, distanceNoiseStdDevBase, distanceNoiseStdDevRisePerMeter));
    }

    void PipelineGraph::ConfigurePcPublisherNode(const AZStd::string& topicName, const AZStd::string& frameId, const ROS2::QoS& qosPolicy)
    {
        const bool FirstConfiguration = !IsPublisherConfigured();

        RGL_CHECK(rgl_node_points_ros2_publish_with_qos(
            &m_nodes.m_pointCloudPublish,
            topicName.c_str(),
            frameId.c_str(),
            static_cast<rgl_qos_policy_reliability_t>(static_cast<int>(qosPolicy.GetQoS().reliability())),
            static_cast<rgl_qos_policy_durability_t>(static_cast<int>(qosPolicy.GetQoS().durability())),
            static_cast<rgl_qos_policy_history_t>(static_cast<int>(qosPolicy.GetQoS().history())),
            qosPolicy.GetQoS().depth()));

        if (FirstConfiguration)
        {
            // clang-format off
            AddConditionalConnection(m_nodes.m_pcPublishFormat, m_nodes.m_pointCloudPublish, [](const PipelineGraph& graph){ return graph.IsPcPublishingEnabled(); });
            // clang-format on
        }
    }

    void PipelineGraph::ConfigureRaytraceNodeNonHits(float minRangeNonHitValue, float maxRangeNonHitValue)
    {
        RGL_CHECK(rgl_node_raytrace_configure_non_hits(m_nodes.m_rayTrace, minRangeNonHitValue, maxRangeNonHitValue));
    }

    void PipelineGraph::SetIsCompactEnabled(bool value)
    {
        SetIsFeatureEnabled(PipelineFeatureFlags::PointsCompact, value);
    }

    void PipelineGraph::SetIsPcPublishingEnabled(bool value)
    {
        if (value && !IsPublisherConfigured())
        {
            AZ_Assert(false, "Trying to enable publishing without the publisher node configured.");
            return;
        }
        SetIsFeatureEnabled(PipelineFeatureFlags::PointCloudPublishing, value);
    }

    void PipelineGraph::SetIsNoiseEnabled(bool value)
    {
        SetIsFeatureEnabled(PipelineFeatureFlags::Noise, value);
    }

    void PipelineGraph::Run()
    {
        RGL_CHECK(rgl_graph_run(m_nodes.m_rayPoses));
    }

    bool PipelineGraph::GetResults(RaycastResults& results) const
    {
        bool success = true;
        for (rgl_field_t field : results.m_fields)
        {
            switch (field)
            {
            case RGL_FIELD_IS_HIT_I32:
                success = success && GetResult(results.m_isHit, RGL_FIELD_IS_HIT_I32);
                break;
            case RGL_FIELD_XYZ_VEC3_F32:
                success = success && GetResult(results.m_xyz, RGL_FIELD_XYZ_VEC3_F32);
                break;
            case RGL_FIELD_DISTANCE_F32:
                success = success && GetResult(results.m_distance, RGL_FIELD_DISTANCE_F32);
                break;
            case RGL_FIELD_INTENSITY_F32:
                success = success && GetResult(results.m_intensity, RGL_FIELD_INTENSITY_F32);
                break;
            default:
                success = false;
                AZ_Assert(false, "Invalid result field type!");
                break;
            }
        }

        return success;
    }

    bool PipelineGraph::IsFeatureEnabled(PipelineGraph::PipelineFeatureFlags feature) const
    {
        return m_activeFeatures & feature;
    }

    void PipelineGraph::SetIsFeatureEnabled(PipelineFeatureFlags feature, bool value)
    {
        if (value)
        {
            m_activeFeatures = static_cast<PipelineFeatureFlags>(m_activeFeatures | feature);
        }
        else
        {
            m_activeFeatures = static_cast<PipelineFeatureFlags>(m_activeFeatures & ~feature);
        }

        UpdateConnections();
    }

    void PipelineGraph::InitializeConditionalConnections()
    {
        const ConditionType NoiseCondition = [](const PipelineGraph& graph)
        {
            return graph.IsNoiseEnabled();
        };

        const ConditionType CompactCondition = [](const PipelineGraph& graph)
        {
            return graph.IsCompactEnabled();
        };

        const ConditionType PublishingCondition = [](const PipelineGraph& graph)
        {
            return graph.IsPcPublishingEnabled();
        };

        // clang-format off
        AddConditionalNode(m_nodes.m_angularNoise, m_nodes.m_lidarTransform, m_nodes.m_rayTrace, NoiseCondition);
        AddConditionalNode(m_nodes.m_distanceNoise, m_nodes.m_rayTrace, m_nodes.m_rayTraceYield, NoiseCondition);
        AddConditionalNode(m_nodes.m_pointsCompact, m_nodes.m_rayTraceYield, m_nodes.m_compactYield, CompactCondition);
        AddConditionalConnection(m_nodes.m_compactYield, m_nodes.m_pointCloudTransform, PublishingCondition);
        // clang-format on
        if (IsPublisherConfigured())
        {
            // clang-format off
            AddConditionalConnection( m_nodes.m_pcPublishFormat, m_nodes.m_pointCloudPublish, PublishingCondition);
            // clang-format on
        }
    }

    void PipelineGraph::UpdateConnections()
    {
        for (ConditionalConnection& connection : m_conditionalConnections)
        {
            connection.Update(*this);
        }
    }

    void PipelineGraph::AddConditionalNode(rgl_node_t node, rgl_node_t parent, rgl_node_t child, const ConditionType& condition)
    {
        AddConditionalConnection(parent, node, condition);
        AddConditionalConnection(node, child, condition);
        // clang-format off
        AddConditionalConnection(parent, child, [condition](const PipelineGraph& pipelineGraph){ return !condition(pipelineGraph); });
        // clang-format on
    }

    void PipelineGraph::AddConditionalConnection(rgl_node_t parent, rgl_node_t child, const ConditionType& condition)
    {
        m_conditionalConnections.emplace_back(parent, child, condition, condition(*this));
    }

    PipelineGraph::ConditionalConnection::ConditionalConnection(
        rgl_node_t parent, rgl_node_t child, const ConditionType& condition, bool activate)
        : m_parent(parent)
        , m_child(child)
        , m_condition(condition)
        , m_isActive(activate)
    {
        if (activate)
        {
            RGL_CHECK(rgl_graph_node_add_child(parent, child));
        }
    }

    void PipelineGraph::ConditionalConnection::Update(const PipelineGraph& graph)
    {
        const bool IsConditionSatisfied = m_condition(graph);
        if (IsConditionSatisfied == m_isActive)
        {
            return;
        }

        m_isActive = IsConditionSatisfied;

        if (IsConditionSatisfied)
        {
            RGL_CHECK(rgl_graph_node_add_child(m_parent, m_child));
            return;
        }

        RGL_CHECK(rgl_graph_node_remove_child(m_parent, m_child));
    }
} // namespace RGL