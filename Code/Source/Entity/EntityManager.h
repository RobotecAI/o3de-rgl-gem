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
#include <AzCore/std/containers/vector.h>
#include <rgl/api/core.h>

namespace RGL
{
    class EntityManager : public AZ::EntityBus::Handler
    {
    public:
        EntityManager(AZ::EntityId entityId);
        EntityManager(const EntityManager& other) = default;
        EntityManager(EntityManager&& other);
        virtual ~EntityManager();

        virtual void Update();

    protected:
        //! Is this Entity static?
        [[nodiscard]] bool IsStatic() const;

        // AZ::EntityBus::Handler implementation overrides
        void OnEntityActivated(const AZ::EntityId& entityId) override;

        //! Updates poses of all RGL entities managed by this EntityManager.
        virtual void UpdatePose();

        AZ::EntityId m_entityId;
        AZStd::vector<rgl_entity_t> m_entities;
    private:
        bool m_isStatic{ false };
    };
} // namespace RGL