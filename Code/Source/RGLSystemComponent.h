/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

#include "Entity/EntityManager.h"
#include "Lidar/LidarSystem.h"
#include "Mesh/MeshLibrary.h"
#include "RGLBus.h"
#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/Math/Vector3.h>
#include <AzFramework/Entity/EntityContextBus.h>

struct Scene;

namespace RGL
{
    class RGLSystemComponent
        : public AZ::Component
        , protected RGLRequestBus::Handler
        , protected AzFramework::EntityContextEventBus::Handler
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
        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Activate() override;
        void Deactivate() override;
        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // RGLRequestBus::Handler implementation
        void ExcludeEntity(const AZ::EntityId& excludedEntityId) override;
        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // AzFramework::EntityContextEventBus::Handler interface implementation
        void OnEntityContextCreateEntity(AZ::Entity& entity) override;
        void OnEntityContextDestroyEntity(const AZ::EntityId& id) override;
        void OnEntityContextReset() override;
        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // AZ::TickBus::Handler interface implementation
        void OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time) override;
        ////////////////////////////////////////////////////////////////////////

    private:
        LidarSystem m_lidarSystem;

        MeshLibrary m_meshLibrary;
        AZStd::set<AZ::EntityId> m_excludedEntities;
        AZStd::unordered_map<AZ::EntityId, EntityManager> m_entityManagers;
    };
} // namespace RGL
