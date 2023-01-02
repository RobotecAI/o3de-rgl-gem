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

#include <AtomLyIntegration/CommonFeatures/Mesh/MeshComponentBus.h>
#include <AzCore/Component/EntityBus.h>
#include <AzCore/Component/EntityId.h>
#include <rgl/api/core.h>

namespace RGL
{
    //! Class used for managing RGL's representation of an Entity with a MeshComponent.
    class EntityManager
        : protected AZ::Render::MeshComponentNotificationBus::Handler
        , protected AZ::EntityBus::Handler
    {
    public:
        EntityManager(AZ::EntityId entityId);
        EntityManager(EntityManager&& entityManager);
        EntityManager(const EntityManager& entityManager) = delete;
        ~EntityManager();

        //! Is this Entity static?
        bool IsStatic() const;

        //! Updates poses of all RGL entities managed by this EntityManager.
        void UpdatePose();

    protected:
        ////////////////////////////////////////////////////////////////////////
        // AZ::Render::MeshComponentNotificationBus::Handler implementation
        void OnModelReady(
            const AZ::Data::Asset<AZ::RPI::ModelAsset>& modelAsset,
            [[maybe_unused]] const AZ::Data::Instance<AZ::RPI::Model>& model) override;
        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // AZ::EntityBus::Handler implementation
        void OnEntityActivated(const AZ::EntityId& entityId) override;
        ////////////////////////////////////////////////////////////////////////
    private:
        bool m_isStatic{ false };
        AZ::EntityId m_entityId;
        AZStd::vector<rgl_entity_t> m_entities;
    };
} // namespace RGL