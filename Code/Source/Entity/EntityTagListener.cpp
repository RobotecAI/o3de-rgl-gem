/* Copyright 2024, Robotec.ai sp. z o.o.
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
#include <Entity/EntityTagListener.h>
#include <RGL/RGLBus.h>

namespace RGL
{
    EntityTagListener::EntityTagListener(AZ::EntityId entityId)
        : m_entityId(entityId)
    {
        AZ::EntityBus::Handler::BusConnect(m_entityId);
    }

    EntityTagListener::~EntityTagListener()
    {
        AZ::EntityBus::Handler::BusDisconnect(m_entityId);
    }

    void EntityTagListener::OnEntityActivated([[maybe_unused]] const AZ::EntityId& entityId)
    {
        LmbrCentral::Tags entityTags;
        LmbrCentral::TagComponentRequestBus::EventResult(entityTags, m_entityId, &LmbrCentral::TagComponentRequests::GetTags);

        if (!entityTags.empty())
        {
            RGLInterface::Get()->ReviseEntityPresence(m_entityId);
        }

        LmbrCentral::TagComponentNotificationsBus::Handler::BusConnect(m_entityId);
    }

    void EntityTagListener::OnEntityDeactivated(const AZ::EntityId& entity_id)
    {
        LmbrCentral::TagComponentNotificationsBus::Handler::BusDisconnect(m_entityId);
    }

    void EntityTagListener::OnTagAdded(const LmbrCentral::Tag& crc32)
    {
        RGLInterface::Get()->ReviseEntityPresence(m_entityId);
    }

    void EntityTagListener::OnTagRemoved(const LmbrCentral::Tag& crc32)
    {
        RGLInterface::Get()->ReviseEntityPresence(m_entityId);
    }
} // namespace RGL
