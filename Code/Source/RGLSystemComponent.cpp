/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#include <RGLSystemComponent.h>

#include <AzCore/Asset/AssetCommon.h>
#include <AzCore/Component/Entity.h>
#include <AzCore/Component/TransformBus.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>

#include <AzFramework/Entity/EntityContext.h>
#include <AzFramework/Entity/EntityContextBus.h>
#include <AzFramework/Entity/GameEntityContextBus.h>

#include <AtomLyIntegration/CommonFeatures/Mesh/MeshComponentConstants.h>
#include <ROS2/Lidar/LidarRaycasterBus.h>
#include <ROS2/ROS2Bus.h>

#include <rgl/api/e2e_extensions.h>
#include <rgl/api/experimental.h>

#include "Utilities/RGLUtils.h"

namespace RGL
{
    void RGLSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<RGLSystemComponent, AZ::Component>()->Version(0);

            if (AZ::EditContext* ec = serializeContext->GetEditContext())
            {
                ec->Class<RGLSystemComponent>("RGLSystemComponent", "[Description of functionality provided by this component]")
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
        : m_gameEntityContextId{ AzFramework::EntityContextId::CreateNull() }
    {
        ErrorCheck(rgl_configure_logging(RGL_LOG_LEVEL_OFF, nullptr, true));
        if (RGLInterface::Get() == nullptr)
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

        AzFramework::GameEntityContextRequestBus::BroadcastResult(
            m_gameEntityContextId, &AzFramework::GameEntityContextRequestBus::Events::GetGameEntityContextId);
        AZ_Assert(!m_gameEntityContextId.IsNull(), "Invalid GameEntityContextId");

        AzFramework::EntityContextEventBus::Handler::BusConnect(m_gameEntityContextId);

        auto* ros2Interface = ROS2::ROS2Interface::Get();
        AZ_Assert(ros2Interface != nullptr, "The ROS2 interface was inaccessable.");
        m_lidarSystem.Activate(ros2Interface->RegisterLidarSystem("RobotecGPULidar"));
    }

    void RGLSystemComponent::Deactivate()
    {
        m_lidarSystem.Deactivate();
        AzFramework::EntityContextEventBus::Handler::BusDisconnect(m_gameEntityContextId);
        AZ::TickBus::Handler::BusDisconnect();

        m_entityManagers.clear();
        ErrorCheck(rgl_cleanup());
    }

    void RGLSystemComponent::ExcludeEntity(const AZ::EntityId& excludedEntityId)
    {
        size_t erased = m_entityManagers.erase(excludedEntityId);
        if (erased == 0UL)
        {
            m_excludedEntities.insert(excludedEntityId);
        }
    }

    // TODO - implement the rest of visible components (if needed)
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
        ErrorCheck(rgl_cleanup());
    }

    void RGLSystemComponent::OnTick(float deltaTime, AZ::ScriptTimePoint time)
    {
        for (auto entityManager = m_entityManagers.begin(); entityManager != m_entityManagers.end(); ++entityManager)
        {
            if (entityManager->second.IsStatic())
            {
                continue;
            }

            entityManager->second.UpdatePose();
        }
    }
} // namespace RGL
