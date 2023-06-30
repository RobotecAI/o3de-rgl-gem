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

#include <rgl/api/core.h>

namespace RGL
{
    //! Class that manages the RGL pipeline graph construction, which depends on
    //! three conditions: point-cloud compact, noise and publication. Below is a
    //! visual representation of the pipeline graph.
    //!
    //!      +--------+    +--------------+    +-?--?--?--?-+
    //!      |rayPoses+---->lidarTransform+---->angularNoise?
    //!      +--------+    +--------------+    +-?--?--?--?++
    //!                                                    |
    //!      +-?--?---?--?-+     +-?--?---?--?-+      +----v---+
    //!      |pointsCompact<-----+distanceNoise<------+rayTrace|
    //!      +-?--?-+-?--?-+     +-?--?-?-?--?-+      +--------+
    //!             |                   |
    //!      +------v-----+      +------v--------+
    //!      |pointsFormat|      |pointsTransform|
    //!      +------------+      +------+--------+
    //!                                 |
    //!                        +--------v----------+
    //!                        |pointsFormatPublish|
    //!                        +--------+----------+
    //!                                 |
    //!                         +-------v---------+
    //!                         |pointCloudPublish|
    //!                         +-----------------+
    //!
    //! Enabling compact adds an additional pointsCompact node to the pipeline,
    //! enabling noise, adds the angularNoise and distance noise nodes whereas
    //! enabling point-cloud publishing adds an additional branch to the pipeline
    //! graph consisting of three nodes: pointsTransform, pointsFormatPublish
    //! and pointCloudPublish.
    class PipelineGraph
    {
    public:
        struct Nodes
        {
            rgl_node_t m_rayPoses{ nullptr }, m_lidarTransform{ nullptr }, m_angularNoise{ nullptr }, m_rayTrace{ nullptr },
                m_distanceNoise{ nullptr }, m_pointsCompact{ nullptr }, m_pointsFormat{ nullptr }, m_pointCloudTransform{ nullptr },
                m_pointsFormatPublish{ nullptr }, m_pointCloudPublish{ nullptr };
        };

        PipelineGraph(float maxRange, AZStd::vector<rgl_field_t>& resultFields);
        PipelineGraph(const PipelineGraph& other) = delete;
        PipelineGraph(PipelineGraph&& other);
        ~PipelineGraph();

        [[nodiscard]] bool IsCompactEnabled() const
        {
            return m_isCompactEnabled;
        };
        [[nodiscard]] bool IsPublishingEnabled() const
        {
            return m_isPublishingEnabled;
        };
        [[nodiscard]] bool IsNoiseEnabled() const
        {
            return m_isNoiseEnabled;
        }
        
        [[nodiscard]] bool IsPublisherConfigured() const
        {
            return m_nodes.m_pointCloudPublish;
        }

        [[nodiscard]] const Nodes& GetNodes() const
        {
            return m_nodes;
        };

        Nodes& GetNodes()
        {
            return m_nodes;
        };
        void SetIsCompactEnabled(bool value);
        void SetIsPublishingEnabled(bool value);
        void SetIsNoiseEnabled(bool value);

    private:
        Nodes m_nodes;
        bool m_isCompactEnabled{ true }, m_isPublishingEnabled{ false }, m_isNoiseEnabled{ false };
    };
} // namespace RGL
