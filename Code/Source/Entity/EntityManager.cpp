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
#include <LmbrCentral/Scripting/TagComponentBus.h>
#include <RGL/RGLBus.h>
#include <ROS2/Lidar/SegmentationUtils.h>
#include <Utilities/RGLUtils.h>

namespace RGL
{
    EntityManager::EntityManager(AZ::EntityId entityId)
        : m_entityId{ entityId }
        , m_segmentationEntityId{ Utils::GenerateSegmentationEntityId() }
    {
    }

    EntityManager::~EntityManager()
    {
        AZ::EntityBus::Handler::BusDisconnect();
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
        AZ::NonUniformScaleRequestBus::Event(
            entityId, &AZ::NonUniformScaleRequests::RegisterScaleChangedEvent, m_nonUniformScaleChangedHandler);
        // Get current non-uniform scale (if there is no non-uniform scale added, the value won't be changed (nullopt))
        AZ::NonUniformScaleRequestBus::EventResult(m_nonUniformScale, entityId, &AZ::NonUniformScaleRequests::GetScale);

        m_isPoseUpdateNeeded = true;
        SetPackedRglEntityId();
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
        for (Wrappers::RglEntity& entity : m_entities)
        {
            entity.SetTransform(entityPoseRgl);
        }

        m_isPoseUpdateNeeded = false;
    }

    void EntityManager::SetPackedRglEntityId()
    {
        m_packedRglEntityId = CalculatePackedRglEntityId();
        for (Wrappers::RglEntity& entity : m_entities)
        {
            entity.SetId(m_packedRglEntityId.value());
        }
    }

    int32_t EntityManager::CalculatePackedRglEntityId() const
    {
        return Utils::PackRglEntityId(
            ROS2::SegmentationIds{ m_segmentationEntityId, ROS2::SegmentationUtils::FetchClassIdForEntity(m_entityId) });
    }
} // namespace RGL
