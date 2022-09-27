/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#include "RGLEditorSystemComponent.h"
#include <AzCore/Serialization/SerializeContext.h>

namespace RGL
{
    void RGLEditorSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<RGLEditorSystemComponent, RGLSystemComponent>()->Version(0);
        }
    }

    RGLEditorSystemComponent::RGLEditorSystemComponent() = default;

    RGLEditorSystemComponent::~RGLEditorSystemComponent() = default;

    void RGLEditorSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        BaseSystemComponent::GetProvidedServices(provided);
        provided.push_back(AZ_CRC_CE("RGLEditorService"));
    }

    void RGLEditorSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        BaseSystemComponent::GetIncompatibleServices(incompatible);
        incompatible.push_back(AZ_CRC_CE("RGLEditorService"));
    }

    void RGLEditorSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        BaseSystemComponent::GetRequiredServices(required);
    }

    void RGLEditorSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        BaseSystemComponent::GetDependentServices(dependent);
    }

    void RGLEditorSystemComponent::Activate()
    {
        RGLSystemComponent::Activate();
    }

    void RGLEditorSystemComponent::Deactivate()
    {
        RGLSystemComponent::Deactivate();
    }

} // namespace RGL
