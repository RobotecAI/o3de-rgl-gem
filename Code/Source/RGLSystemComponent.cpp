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

#include <AtomLyIntegration/CommonFeatures/Mesh/MeshComponentConstants.h>
#include <AzCore/Component/TickBus.h>
#include <AzFramework/Entity/EntityContext.h>
#include <AzFramework/Entity/GameEntityContextBus.h>
#include <Entity/ActorEntityManager.h>
#include <Entity/EntityManager.h>
#include <Entity/MeshEntityManager.h>
#include <Integration/Components/ActorComponent.h>
#include <RGLSystemComponent.h>
#include <Utilities/RGLUtils.h>

namespace RGL
{
    void RGLSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<RGLSystemComponent, AZ::Component>()->Version(0);

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<RGLSystemComponent>("RGLSystemComponent", "[Description of functionality provided by this component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true);
            }
        }
    }

    void RGLSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("RGLService"));
    }

    void RGLSystemComponent::GetIncompatibleServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("RGLService"));
    }

    void RGLSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("ROS2Service"));
    }

    void RGLSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    RGLSystemComponent::RGLSystemComponent()
    {
        RGL_CHECK(rgl_configure_logging(RGL_LOG_LEVEL_WARN, nullptr, true));
        if (!RGLInterface::Get())
        {
            RGLInterface::Register(this);
        }
    }

    RGLSystemComponent::~RGLSystemComponent()
    {
        if (RGLInterface::Get() == this)
        {
            RGLInterface::Unregister(this);
        }
    }

    void RGLSystemComponent::Activate()
    {
        AzFramework::EntityContextId gameEntityContextId;
        AzFramework::GameEntityContextRequestBus::BroadcastResult(
            gameEntityContextId, &AzFramework::GameEntityContextRequestBus::Events::GetGameEntityContextId);
        AZ_Assert(!gameEntityContextId.IsNull(), "Invalid GameEntityContextId");

        AzFramework::EntityContextEventBus::Handler::BusConnect(gameEntityContextId);
        LidarSystemNotificationBus::Handler::BusConnect();

        m_rglLidarSystem.Activate();
    }

    void RGLSystemComponent::Deactivate()
    {
        m_rglLidarSystem.Deactivate();
        LidarSystemNotificationBus::Handler::BusDisconnect();
        AzFramework::EntityContextEventBus::Handler::BusDisconnect();

        m_entityManagers.clear();
        m_modelLibrary.Clear();
        m_rglLidarSystem.Clear();
    }

    void RGLSystemComponent::ExcludeEntity(const AZ::EntityId& excludedEntityId)
    {
        if (!m_entityManagers.erase(excludedEntityId))
        {
            m_excludedEntities.insert(excludedEntityId);
        }
    }

    void RGLSystemComponent::SetSceneConfiguration(const SceneConfiguration& config)
    {
        m_sceneConfig = config;
        RGLNotificationBus::Broadcast(&RGLNotifications::OnSceneConfigurationSet, config);
    }

    const SceneConfiguration& RGLSystemComponent::GetSceneConfiguration() const
    {
        return m_sceneConfig;
    }

    void RGLSystemComponent::OnEntityContextCreateEntity(AZ::Entity& entity)
    {
        if (m_excludedEntities.contains(entity.GetId()))
        {
            return;
        }

        if (m_activeLidarCount < 1U)
        {
            m_unprocessedEntities.emplace(entity.GetId());
            return;
        }

        ProcessEntity(entity);
    }

    void RGLSystemComponent::OnEntityContextDestroyEntity(const AZ::EntityId& id)
    {
        m_unprocessedEntities.erase(id);
        m_entityManagers.erase(id);
    }

    void RGLSystemComponent::OnEntityContextReset()
    {
        m_entityManagers.clear();
        m_unprocessedEntities.clear();
        m_modelLibrary.Clear();
        m_rglLidarSystem.Clear();
    }

    void RGLSystemComponent::OnLidarCreated()
    {
        ++m_activeLidarCount;

        if (m_activeLidarCount > 1U)
        {
            return;
        }

        RGLNotificationBus::Broadcast(&RGLNotifications::OnAnyLidarExists);
        for (auto entityIdIt = m_unprocessedEntities.begin(); entityIdIt != m_unprocessedEntities.end();)
        {
            AZ::Entity* entity = nullptr;
            AZ::ComponentApplicationBus::BroadcastResult(entity, &AZ::ComponentApplicationRequests::FindEntity, *entityIdIt);
            AZ_Assert(entity, "Failed to find entity with provided id!");
            ProcessEntity(*entity);
            entityIdIt = m_unprocessedEntities.erase(entityIdIt);
        }
    }

    void RGLSystemComponent::OnLidarDestroyed()
    {
        --m_activeLidarCount;

        if (m_activeLidarCount > 0U)
        {
            return;
        }

        RGLNotificationBus::Broadcast(&RGLNotifications::OnNoLidarExists);
        for (auto& m_entityManager : m_entityManagers)
        {
            m_unprocessedEntities.emplace(m_entityManager.first);
        }
        m_entityManagers.clear();
        m_modelLibrary.Clear();
    }

    void RGLSystemComponent::ProcessEntity(const AZ::Entity& entity)
    {
        AZStd::unique_ptr<EntityManager> entityManager;
        if (entity.FindComponent<EMotionFX::Integration::ActorComponent>())
        {
            entityManager = AZStd::make_unique<ActorEntityManager>(entity.GetId());
        }
        else if (entity.FindComponent(AZ::Render::MeshComponentTypeId))
        {
            entityManager = AZStd::make_unique<MeshEntityManager>(entity.GetId());
        }
        else
        {
            return;
        }

        [[maybe_unused]] bool inserted = m_entityManagers.emplace(entity.GetId(), AZStd::move(entityManager)).second;
        AZ_Error(__func__, inserted, "Object with provided entityId already exists.");
    }

    void RGLSystemComponent::UpdateScene()
    {
        AZ::ScriptTimePoint currentTime;
        AZ::TickRequestBus::BroadcastResult(currentTime, &AZ::TickRequestBus::Events::GetTimeAtCurrentTick);
        // Skip if already updated
        if (m_sceneUpdateLastTime.Get() == currentTime.Get())
        {
            return;
        }
        m_sceneUpdateLastTime = currentTime;

        for (auto&& [entityId, entityManager] : m_entityManagers)
        {
            entityManager->Update();
        }
    }
} // namespace RGL
