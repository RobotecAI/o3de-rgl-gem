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
#include <Wrappers/RglEntity.h>
#include <rgl/api/core.h>

namespace RGL
{
    //! Base class for Entity Manager.
    //! Although it already implements the EntityBus handler,
    //! the derived classes have to handle bus connection
    //! through BusConnect and BusDisconnect function calls.
    //! This is to allow for further overrides.
    class EntityManager : public AZ::EntityBus::Handler
    {
    public:
        explicit EntityManager(AZ::EntityId entityId);
        EntityManager(const EntityManager& other) = delete;
        EntityManager(EntityManager&& other) = delete;
        EntityManager& operator=(EntityManager&& rhs) = delete;
        EntityManager& operator=(const EntityManager&) = delete;
        virtual ~EntityManager();

        virtual void Update();

    protected:
        // AZ::EntityBus::Handler implementation overrides
        void OnEntityActivated(const AZ::EntityId& entityId) override;
        void OnEntityDeactivated(const AZ::EntityId& entityId) override;

        //! Updates poses of all RGL entities managed by this EntityManager.
        virtual void UpdatePose();

        AZ::EntityId m_entityId;
        AZStd::vector<Wrappers::RglEntity> m_entities;
        bool m_isPoseUpdateNeeded{ false };

    private:
        // clang-format off
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
        // clang-format on

        AZ::Transform m_worldTm{ AZ::Transform::CreateIdentity() };
        AZStd::optional<AZ::Vector3> m_nonUniformScale{ AZStd::nullopt };
    };
} // namespace RGL