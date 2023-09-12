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
#include <Entity/TerrainEntityManagerEditorSystemComponent.h>
#include <RGLEditorSystemComponent.h>
#include <RGLModuleInterface.h>
#include <SceneConfigurationComponent.h>

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
            // This will associate the AzTypeInfo information for the components with the SerializeContext, BehaviorContext and
            // EditContext. This happens through the [MyComponent]::Reflect() function.
            m_descriptors.insert(
                m_descriptors.end(),
                {
                    RGLEditorSystemComponent::CreateDescriptor(),
                    TerrainEntityManagerEditorSystemComponent::CreateDescriptor(),
                    SceneConfigurationComponent::CreateDescriptor(),
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
                azrtti_typeid<SceneConfigurationComponent>(),
            };
        }
    };
} // namespace RGL

AZ_DECLARE_MODULE_CLASS(Gem_RGL, RGL::RGLEditorModule)
