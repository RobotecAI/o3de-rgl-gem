/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Module/Module.h>
#include <RGLSystemComponent.h>

namespace RGL
{
    class RGLModuleInterface : public AZ::Module
    {
    public:
        AZ_RTTI(RGLModuleInterface, "{75F40506-8366-49E2-9E49-F94010953EE3}", AZ::Module);
        AZ_CLASS_ALLOCATOR(RGLModuleInterface, AZ::SystemAllocator, 0);

        RGLModuleInterface()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            // Add ALL components descriptors associated with this gem to m_descriptors.
            // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and
            // EditContext. This happens through the [MyComponent]::Reflect() function.
            m_descriptors.insert(
                m_descriptors.end(),
                {
                    RGLSystemComponent::CreateDescriptor(),
                });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList{
                azrtti_typeid<RGLSystemComponent>(),
            };
        }
    };
} // namespace RGL
