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

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/Math/Vector3.h>
#include <AzCore/Script/ScriptTimePoint.h>
#include <AzFramework/Entity/EntityContextBus.h>
#include <Entity/EntityTagListener.h>
#include <Lidar/LidarSystem.h>
#include <Lidar/LidarSystemNotificationBus.h>
#include <LmbrCentral/Scripting/TagComponentBus.h>
#include <Model/ModelLibrary.h>
#include <RGL/RGLBus.h>

namespace RGL
{
    class EntityManager;

    class RGLSystemComponent
        : public AZ::Component
        , protected RGLRequestBus::Handler
        , protected AzFramework::EntityContextEventBus::Handler
        , protected LidarSystemNotificationBus::Handler
        , protected AZ::TickBus::Handler
    {
    public:
        AZ_COMPONENT(RGL::RGLSystemComponent, "{dbd5b1c5-249f-4eca-a142-2533ebe7f680}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        RGLSystemComponent();
        ~RGLSystemComponent() override;

    protected:
        // AZ::Component overrides
        void Activate() override;
        void Deactivate() override;

        // RGLRequestBus overrides
        void ExcludeEntity(const AZ::EntityId& excludedEntityId) override;
        void SetSceneConfiguration(const SceneConfiguration& config) override;
        [[nodiscard]] const SceneConfiguration& GetSceneConfiguration() const override;
        void UpdateScene() override;
        void ReviseEntityPresence(AZ::EntityId entityId) override;

        // AzFramework::EntityContextEventBus overrides
        void OnEntityContextCreateEntity(AZ::Entity& entity) override;
        void OnEntityContextDestroyEntity(const AZ::EntityId& id) override;
        void OnEntityContextReset() override;

        // LidarNotificationBus overides
        void OnLidarCreated() override;
        void OnLidarDestroyed() override;

        // AZ::TickBus overrides
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

    private:
        static bool HasVisuals(const AZ::Entity& entity);
        bool ShouldEntityBeExcluded(AZ::EntityId entityId) const;
        void ProcessEntity(const AZ::Entity& entity);
        void UpdateTagExcludedEntities();

        SceneConfiguration m_sceneConfig;
        LidarSystem m_rglLidarSystem;
        ModelLibrary m_modelLibrary;

        AZStd::set<AZ::EntityId> m_excludedEntities;
        AZStd::set<AZ::EntityId> m_unprocessedEntities;
        AZStd::vector<AZ::EntityId> m_managersToBeRemoved;
        AZStd::unordered_map<AZ::EntityId, AZStd::unique_ptr<EntityManager>> m_entityManagers;
        AZStd::vector<EntityTagListener> m_entityTagListeners;

        AZStd::vector<LmbrCentral::Tag> m_excludedTags;

        AZ::ScriptTimePoint m_sceneUpdateLastTime{};
        size_t m_activeLidarCount{};
    };
} // namespace RGL
