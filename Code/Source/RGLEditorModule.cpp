/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#include "RGLEditorSystemComponent.h"
#include "Entity/TerrainEntityManagerEditorSystemComponent.h"
#include <RGLModuleInterface.h>

namespace RGL
{
    class RGLEditorModule : public RGLModuleInterface
    {
    public:
        AZ_RTTI(RGLEditorModule, "{06ADDE3E-333C-49E7-A63F-BE3E8F3CFB4C}", RGLModuleInterface);
        AZ_CLASS_ALLOCATOR(RGLEditorModule, AZ::SystemAllocator, 0);

        RGLEditorModule()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            // Add ALL components descriptors associated with this gem to m_descriptors.
            // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and
            // EditContext. This happens through the [MyComponent]::Reflect() function.
            m_descriptors.insert(
                m_descriptors.end(),
                {
                    RGLEditorSystemComponent::CreateDescriptor(),
                    TerrainEntityManagerEditorSystemComponent::CreateDescriptor(),
                });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         * Non-SystemComponents should not be added here
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList{
                azrtti_typeid<RGLEditorSystemComponent>(),
                azrtti_typeid<TerrainEntityManagerEditorSystemComponent>(),
            };
        }
    };
} // namespace RGL

AZ_DECLARE_MODULE_CLASS(Gem_RGL, RGL::RGLEditorModule)
