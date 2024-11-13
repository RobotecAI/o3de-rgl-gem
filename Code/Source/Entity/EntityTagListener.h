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
#pragma once

#include <AzCore/Component/EntityBus.h>
#include <LmbrCentral/Scripting/TagComponentBus.h>

namespace RGL
{
    class EntityTagListener
        : AZ::EntityBus::Handler
        , LmbrCentral::TagComponentNotificationsBus::Handler
    {
    public:
        EntityTagListener(AZ::EntityId entityId);
        ~EntityTagListener();

        void OnEntityActivated(const AZ::EntityId&) override;
        void OnEntityDeactivated(const AZ::EntityId&) override;

        // TagComponentNotificationsBus overrides
        void OnTagAdded(const LmbrCentral::Tag&) override;
        void OnTagRemoved(const LmbrCentral::Tag&) override;

    private:
        AZ::EntityId m_entityId;
    };
} // namespace RGL