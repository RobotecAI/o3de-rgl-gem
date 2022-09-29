/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

#include "rgl/api/experimental.h"
#include <AtomLyIntegration/CommonFeatures/Mesh/MeshComponentBus.h>
#include <AzCore/Component/EntityBus.h>
#include <AzCore/Component/EntityId.h>

namespace RGL
{
    //! Manages RGL's representation of an Entity with a MeshComponent.
    class EntityManager
        : protected AZ::Render::MeshComponentNotificationBus::Handler
        , protected AZ::EntityBus::Handler
    {
    public:
        EntityManager(AZ::EntityId entityId);
        ~EntityManager();

        //! Is this Entity static?
        //! @return A boolean value informing the callee whether this entity is static.
        bool IsStatic();

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
        bool m_isStatic;
        AZ::EntityId m_entityId;
        AZStd::vector<rgl_entity_t> m_entities;
    };
} // namespace RGL