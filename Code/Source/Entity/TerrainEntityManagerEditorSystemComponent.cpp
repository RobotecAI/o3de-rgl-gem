/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "TerrainEntityManagerEditorSystemComponent.h"
#include <AzCore/Serialization/SerializeContext.h>

namespace RGL
{
    void TerrainEntityManagerEditorSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<TerrainEntityManagerEditorSystemComponent, TerrainEntityManagerSystemComponent>()->Version(0);
        }
    }

    void TerrainEntityManagerEditorSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        BaseSystemComponent::GetProvidedServices(provided);
        provided.push_back(AZ_CRC_CE("TerrainEntityManagerEditorService"));
    }

    void TerrainEntityManagerEditorSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        BaseSystemComponent::GetIncompatibleServices(incompatible);
        incompatible.push_back(AZ_CRC_CE("TerrainEntityManagerEditorService"));
    }

    void TerrainEntityManagerEditorSystemComponent::GetRequiredServices(
        [[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        BaseSystemComponent::GetRequiredServices(required);
    }

    void TerrainEntityManagerEditorSystemComponent::GetDependentServices(
        [[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        BaseSystemComponent::GetDependentServices(dependent);
    }

    void TerrainEntityManagerEditorSystemComponent::Activate()
    {
        TerrainEntityManagerSystemComponent::Activate();
    }

    void TerrainEntityManagerEditorSystemComponent::Deactivate()
    {
        TerrainEntityManagerSystemComponent::Deactivate();
    }
} // namespace RGL
