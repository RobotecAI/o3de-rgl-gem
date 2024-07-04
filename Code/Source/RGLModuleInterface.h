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
#pragma once

#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Module/Module.h>
#include <Entity/Terrain/TerrainEntityManagerSystemComponent.h>
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
                    TerrainEntityManagerSystemComponent::CreateDescriptor(),
                });
        }

        //! Add required SystemComponents to the SystemEntity.
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList{
                azrtti_typeid<RGLSystemComponent>(),
                azrtti_typeid<TerrainEntityManagerSystemComponent>(),
            };
        }
    };
} // namespace RGL
