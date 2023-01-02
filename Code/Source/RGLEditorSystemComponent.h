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

#include <RGLSystemComponent.h>

namespace RGL
{
    /// System component for RGL editor
    class RGLEditorSystemComponent : public RGLSystemComponent
    {
        using BaseSystemComponent = RGLSystemComponent;

    public:
        AZ_COMPONENT(RGLEditorSystemComponent, "{E36B695E-36C5-4162-BF86-EA68AA21217C}", BaseSystemComponent);
        static void Reflect(AZ::ReflectContext* context);

        RGLEditorSystemComponent();
        ~RGLEditorSystemComponent();

    private:
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        // AZ::Component
        void Activate() override;
        void Deactivate() override;
    };
} // namespace RGL
