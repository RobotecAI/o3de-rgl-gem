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
#include <AzFramework/Entity/EntityContext.h>
#include <AzFramework/Entity/GameEntityContextBus.h>
#include <RGLSystemComponent.h>
#include <Utilities/RGLUtils.h>
// TODO - Format the code past o3de hash 57a500f3eaf2b0d8450003dd04016f5147e940a7. Missing includes inside the file.
#include <AtomLyIntegration/CommonFeatures/Mesh/MeshComponentConstants.h>

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
        Utils::ErrorCheck(rgl_configure_logging(RGL_LOG_LEVEL_WARN, nullptr, true));
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
        AZ::TickBus::Handler::BusConnect();

        AzFramework::EntityContextId gameEntityContextId;
        AzFramework::GameEntityContextRequestBus::BroadcastResult(
            gameEntityContextId, &AzFramework::GameEntityContextRequestBus::Events::GetGameEntityContextId);
        AZ_Assert(!gameEntityContextId.IsNull(), "Invalid GameEntityContextId");

        AzFramework::EntityContextEventBus::Handler::BusConnect(gameEntityContextId);

        m_rglLidarSystem.Activate();
    }

    void RGLSystemComponent::Deactivate()
    {
        m_rglLidarSystem.Deactivate();
        AzFramework::EntityContextEventBus::Handler::BusDisconnect();
        AZ::TickBus::Handler::BusDisconnect();

        m_entityManagers.clear();
        m_meshLibrary.Clear();
        m_rglLidarSystem.Clear();
        Utils::ErrorCheck(rgl_cleanup());
    }

    void RGLSystemComponent::ExcludeEntity(const AZ::EntityId& excludedEntityId)
    {
        if (!m_entityManagers.erase(excludedEntityId))
        {
            m_excludedEntities.insert(excludedEntityId);
        }
    }

    void RGLSystemComponent::OnEntityContextCreateEntity(AZ::Entity& entity)
    {
        if (entity.FindComponent(AZ::Render::MeshComponentTypeId) == nullptr || m_excludedEntities.contains(entity.GetId()))
        {
            return;
        }

        [[maybe_unused]] bool inserted = m_entityManagers.emplace(entity.GetId(), EntityManager{ entity.GetId() }).second;
        AZ_Error(__func__, inserted, "Object with provided entityId already exists.");
    }

    void RGLSystemComponent::OnEntityContextDestroyEntity(const AZ::EntityId& id)
    {
        m_entityManagers.erase(id);
    }

    void RGLSystemComponent::OnEntityContextReset()
    {
        m_entityManagers.clear();
        m_meshLibrary.Clear();
        m_rglLidarSystem.Clear();
        Utils::ErrorCheck(rgl_cleanup());
    }

    void RGLSystemComponent::OnTick(float deltaTime, AZ::ScriptTimePoint time)
    {
        for (auto& [entityId, entityManager] : m_entityManagers)
        {
            if (entityManager.IsStatic())
            {
                continue;
            }

            entityManager.UpdatePose();
        }
    }
} // namespace RGL
