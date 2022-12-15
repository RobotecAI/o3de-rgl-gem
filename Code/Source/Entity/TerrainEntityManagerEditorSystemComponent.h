/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include "TerrainEntityManagerSystemComponent.h"

namespace RGL
{
    class TerrainEntityManagerEditorSystemComponent : public TerrainEntityManagerSystemComponent
    {
        using BaseSystemComponent = TerrainEntityManagerSystemComponent;

    public:
        AZ_COMPONENT(
            TerrainEntityManagerEditorSystemComponent, "{a5b7f4dc-82bc-48c7-ab57-c18bc7225886}", TerrainEntityManagerSystemComponent);
        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        TerrainEntityManagerEditorSystemComponent() = default;
        ~TerrainEntityManagerEditorSystemComponent() = default;

    private:
        void Activate() override;
        void Deactivate() override;
    };
} // namespace RGL
