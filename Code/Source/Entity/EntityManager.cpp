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

#include <Entity/EntityManager.h>
#include <Utilities/RGLUtils.h>

namespace RGL
{
    AZStd::atomic_int32_t EntityManager::m_compresedIdCounter = { 0 };

    EntityManager::EntityManager(AZ::EntityId entityId, AZStd::set<AZStd::pair<AZStd::string,uint8_t>> &class_tags)
        : m_entityId{ entityId }
    {
        m_compresedId = m_compresedIdCounter.fetch_add(1);
        m_compresedId = m_compresedId%(1<<COMPRESSED_ID_BIT_DEPTH);
        
        for (const auto& tag : class_tags)
        {
            LmbrCentral::Tag tag_to_test(tag.first);
            bool has_tag=false;
            LmbrCentral::TagComponentRequestBus::EventResult(has_tag,m_entityId, &LmbrCentral::TagComponentRequests::HasTag,tag_to_test);
            if (has_tag)
            {
                if (m_classId != 0)
                {
                    AZ_Warning("EntityManager", false, "Entity with ID: %s has more than one class tag. Assigning tag: %s", m_entityId.ToString().c_str(), tag.first.c_str());
                }
                m_classId = tag.second;
            }
        }
        AZ::EntityBus::Handler::BusConnect(m_entityId);
    }

    EntityManager::~EntityManager()
    {
        AZ::EntityBus::Handler::BusDisconnect();

        for (rgl_entity_t entity : m_entities)
        {
            RGL_CHECK(rgl_entity_destroy(entity));
        }
    }

    void EntityManager::Update()
    {
        if (!m_isPoseUpdateNeeded)
        {
            return;
        }

        UpdatePose();
    }

    void EntityManager::OnEntityActivated(const AZ::EntityId& entityId)
    {
        //// Transform
        // Register transform changed event handler
        AZ::TransformBus::Event(entityId, &AZ::TransformBus::Events::BindTransformChangedEventHandler, m_transformChangedHandler);
        // Get current transform
        AZ::TransformBus::EventResult(m_worldTm, entityId, &AZ::TransformBus::Events::GetWorldTM);

        //// Non-uniform scale
        // Register non-uniform scale changed event handler
        AZ::NonUniformScaleRequestBus::Event(entityId, &AZ::NonUniformScaleRequests::RegisterScaleChangedEvent, m_nonUniformScaleChangedHandler);
        // Get current non-uniform scale (if there is no non-uniform scale added, the value won't be changed (nullopt))
        AZ::NonUniformScaleRequestBus::EventResult(m_nonUniformScale, entityId, &AZ::NonUniformScaleRequests::GetScale);

        m_isPoseUpdateNeeded = true;
    }

    void EntityManager::OnEntityDeactivated(const AZ::EntityId& entityId)
    {
        m_transformChangedHandler.Disconnect();
        m_nonUniformScaleChangedHandler.Disconnect();
    }

    void EntityManager::UpdatePose()
    {
        if (m_entities.empty())
        {
            m_isPoseUpdateNeeded = false;
            return;
        }

        AZ::Matrix3x4 transform3x4f = AZ::Matrix3x4::CreateFromTransform(m_worldTm);
        if (m_nonUniformScale.has_value())
        {
            transform3x4f *= AZ::Matrix3x4::CreateScale(m_nonUniformScale.value());
        }
        const rgl_mat3x4f entityPoseRgl = Utils::RglMat3x4FromAzMatrix3x4(transform3x4f);
        for (rgl_entity_t entity : m_entities)
        {
            RGL_CHECK(rgl_entity_set_pose(entity, &entityPoseRgl));
        }
        m_isPoseUpdateNeeded = false;
    }
} // namespace RGL