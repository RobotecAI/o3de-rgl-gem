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

namespace RGL
{
    PipelineGraph::PipelineGraph(float maxRange, AZStd::vector<rgl_field_t>& resultFields)
    {
        RGL_CHECK(rgl_node_rays_from_mat3x4f(&m_nodes.m_rayPoses, &Utils::IdentityTransform, 1));
        RGL_CHECK(rgl_node_rays_transform(&m_nodes.m_lidarTransform, &Utils::IdentityTransform));
        RGL_CHECK(rgl_node_gaussian_noise_angular_ray(&m_nodes.m_angularNoise, 0.0f, 0.0f, RGL_AXIS_Z));
        RGL_CHECK(rgl_node_raytrace(&m_nodes.m_rayTrace, nullptr, maxRange));
        RGL_CHECK(rgl_node_gaussian_noise_distance(&m_nodes.m_distanceNoise, 0.0f, 0.0f, 0.0f));
        RGL_CHECK(rgl_node_points_compact(&m_nodes.m_pointsCompact));
        RGL_CHECK(rgl_node_points_format(&m_nodes.m_pointsFormat, resultFields.data(), aznumeric_cast<int32_t>(resultFields.size())));

        RGL_CHECK(rgl_node_points_transform(&m_nodes.m_pointCloudTransform, &Utils::IdentityTransform));
        RGL_CHECK(rgl_node_points_format(&m_nodes.m_pointsFormatPublish, resultFields.data(), aznumeric_cast<int32_t>(resultFields.size())));

        RGL_CHECK(rgl_graph_node_add_child(m_nodes.m_rayPoses, m_nodes.m_lidarTransform));
        RGL_CHECK(rgl_graph_node_add_child(m_nodes.m_lidarTransform, m_nodes.m_rayTrace));
        RGL_CHECK(rgl_graph_node_add_child(m_nodes.m_rayTrace, m_nodes.m_pointsCompact));
        RGL_CHECK(rgl_graph_node_add_child(m_nodes.m_pointsCompact, m_nodes.m_pointsFormat));

        RGL_CHECK(rgl_graph_node_add_child(m_nodes.m_pointCloudTransform, m_nodes.m_pointsFormatPublish));
    }

    PipelineGraph::PipelineGraph(PipelineGraph&& other)
        : m_nodes{ other.m_nodes }
        , m_isPublishingEnabled{ other.m_isPublishingEnabled }
        , m_isCompactEnabled{ other.m_isCompactEnabled }
    {
        other.m_nodes = {};
    }

    PipelineGraph::~PipelineGraph()
    {
        if (m_nodes.m_rayPoses)
        {
            RGL_CHECK(rgl_graph_destroy(m_nodes.m_rayTrace));

            if (!IsCompactEnabled())
            {
                RGL_CHECK(rgl_graph_destroy(m_nodes.m_pointsCompact));
            }

            if (!IsPublishingEnabled())
            {
                if (m_nodes.m_pointCloudPublish)
                {
                    RGL_CHECK(rgl_graph_destroy(m_nodes.m_pointCloudPublish));
                }

                RGL_CHECK(rgl_graph_destroy(m_nodes.m_pointsFormatPublish));
            }

            if (!IsNoiseEnabled())
            {
                if (m_nodes.m_angularNoise)
                {
                    RGL_CHECK(rgl_graph_destroy(m_nodes.m_angularNoise));
                }

                if (m_nodes.m_distanceNoise)
                {
                    RGL_CHECK(rgl_graph_destroy(m_nodes.m_distanceNoise));
                }
            }
        }
    }

    void PipelineGraph::SetIsCompactEnabled(bool value)
    {
        if (IsCompactEnabled() == value) // Make sure to only allow state switching calls
        {
            return;
        }

        rgl_node_t compactNodeParent = IsNoiseEnabled() ? m_nodes.m_distanceNoise : m_nodes.m_rayTrace;

        if (value)
        {
            RGL_CHECK(rgl_graph_node_remove_child(compactNodeParent, m_nodes.m_pointsFormat));
            RGL_CHECK(rgl_graph_node_add_child(compactNodeParent, m_nodes.m_pointsCompact));
            RGL_CHECK(rgl_graph_node_add_child(m_nodes.m_pointsCompact, m_nodes.m_pointsFormat));
        }
        else
        {
            RGL_CHECK(rgl_graph_node_remove_child(compactNodeParent, m_nodes.m_pointsCompact));
            RGL_CHECK(rgl_graph_node_remove_child(m_nodes.m_pointsCompact, m_nodes.m_pointsFormat));
            RGL_CHECK(rgl_graph_node_add_child(compactNodeParent, m_nodes.m_pointsFormat));
        }

        m_isCompactEnabled = value;
    }

    void PipelineGraph::SetIsPublishingEnabled(bool value)
    {
        if (IsPublishingEnabled() == value) // Make sure to only allow state switching calls
        {
            return;
        }

        rgl_node_t pointsTransformNodeParent = IsNoiseEnabled() ? m_nodes.m_distanceNoise : m_nodes.m_rayTrace;

        if (value)
        {
            RGL_CHECK(rgl_graph_node_add_child(pointsTransformNodeParent, m_nodes.m_pointCloudTransform));
            RGL_CHECK(rgl_graph_node_add_child(m_nodes.m_pointsFormatPublish, m_nodes.m_pointCloudPublish));
        }
        else
        {
            RGL_CHECK(rgl_graph_node_remove_child(pointsTransformNodeParent, m_nodes.m_pointCloudTransform));
            RGL_CHECK(rgl_graph_node_remove_child(m_nodes.m_pointsFormatPublish, m_nodes.m_pointCloudPublish));
        }

        m_isPublishingEnabled = value;
    }

    void PipelineGraph::SetIsNoiseEnabled(bool value)
    {
        if (IsNoiseEnabled() == value)
        {
            return;
        }

        rgl_node_t distanceNoiseChildNode = IsCompactEnabled() ? m_nodes.m_pointsCompact : m_nodes.m_pointsFormat;

        if (value)
        {
            // Insert the angularNoise node
            RGL_CHECK(rgl_graph_node_remove_child(m_nodes.m_lidarTransform, m_nodes.m_rayTrace));
            RGL_CHECK(rgl_graph_node_add_child(m_nodes.m_lidarTransform, m_nodes.m_angularNoise));
            RGL_CHECK(rgl_graph_node_add_child(m_nodes.m_angularNoise, m_nodes.m_rayTrace));

            // Insert the distanceNoise node
            if (IsPublishingEnabled())
            {
                RGL_CHECK(rgl_graph_node_remove_child(m_nodes.m_rayTrace, m_nodes.m_pointCloudTransform));
                RGL_CHECK(rgl_graph_node_add_child(m_nodes.m_distanceNoise, m_nodes.m_pointCloudTransform));
            }

            RGL_CHECK(rgl_graph_node_remove_child(m_nodes.m_rayTrace, distanceNoiseChildNode));
            RGL_CHECK(rgl_graph_node_add_child(m_nodes.m_rayTrace, m_nodes.m_distanceNoise));
            RGL_CHECK(rgl_graph_node_add_child(m_nodes.m_distanceNoise, distanceNoiseChildNode));
        }
        else
        {
            // Remove the angularNoise node
            RGL_CHECK(rgl_graph_node_remove_child(m_nodes.m_lidarTransform, m_nodes.m_angularNoise));
            RGL_CHECK(rgl_graph_node_remove_child(m_nodes.m_angularNoise, m_nodes.m_rayTrace));
            RGL_CHECK(rgl_graph_node_add_child(m_nodes.m_lidarTransform, m_nodes.m_rayTrace));

            // Remove the distanceNoise node
            if (IsPublishingEnabled())
            {
                RGL_CHECK(rgl_graph_node_remove_child(m_nodes.m_distanceNoise, m_nodes.m_pointCloudTransform));
                RGL_CHECK(rgl_graph_node_add_child(m_nodes.m_rayTrace, m_nodes.m_pointCloudTransform));
            }

            RGL_CHECK(rgl_graph_node_remove_child(m_nodes.m_rayTrace, m_nodes.m_distanceNoise));
            RGL_CHECK(rgl_graph_node_remove_child(m_nodes.m_distanceNoise, distanceNoiseChildNode));
            RGL_CHECK(rgl_graph_node_add_child(m_nodes.m_rayTrace, distanceNoiseChildNode));
        }

        m_isNoiseEnabled = value;
    }
} // namespace RGL