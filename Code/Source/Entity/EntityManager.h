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

#include <AzCore/Component/EntityBus.h>
#include <AzCore/Component/EntityId.h>
#include <AzCore/Component/NonUniformScaleBus.h>
#include <AzCore/Component/TransformBus.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/std/optional.h>
#include <rgl/api/core.h>

namespace RGL
{
    class EntityManager : public AZ::EntityBus::Handler
    {
    public:
        explicit EntityManager(AZ::EntityId entityId, AZStd::set<AZStd::pair<AZStd::string, uint8_t> > &class_tags);

        EntityManager(const EntityManager& other) = delete;
        EntityManager(EntityManager&& other) = delete;
        EntityManager& operator=(EntityManager&& rhs) = delete;
        EntityManager& operator=(const EntityManager&) = delete;
        virtual ~EntityManager();

        virtual void Update();

        // Unique id of the class 0 is the unknown class
        uint8_t m_classId{0};
        
    protected:

        // AZ::EntityBus::Handler implementation overrides
        void OnEntityActivated(const AZ::EntityId& entityId) override;
        void OnEntityDeactivated(const AZ::EntityId& entityId) override;

        //! Updates poses of all RGL entities managed by this EntityManager.
        virtual void UpdatePose();

        AZ::EntityId m_entityId;
        AZStd::vector<rgl_entity_t> m_entities;
        bool m_isPoseUpdateNeeded{ false };
        int32_t get_rgl_id() const { return static_cast<int32_t>(m_classId) << COMPRESSED_ID_BIT_DEPTH | m_compresedId;}
    private:

        AZ::TransformChangedEvent::Handler m_transformChangedHandler{[this](
            [[maybe_unused]] const AZ::Transform& local, const AZ::Transform& world)
            {
                m_worldTm = world;
                m_isPoseUpdateNeeded = true;
            }};

        AZ::NonUniformScaleChangedEvent::Handler m_nonUniformScaleChangedHandler{[this](
                const AZ::Vector3& scale)
            {
                m_nonUniformScale = scale;
                m_isPoseUpdateNeeded = true;
            }};

        AZ::Transform m_worldTm{ AZ::Transform::CreateIdentity() };
        AZStd::optional<AZ::Vector3> m_nonUniformScale{ AZStd::nullopt };




        // Temporary solution to generate unique ids for RGL entities
        static constexpr uint8_t COMPRESSED_ID_BIT_DEPTH = 20;
        int32_t m_compresedId{0};
        static AZStd::atomic_int32_t m_compresedIdCounter;
    };
} // namespace RGL