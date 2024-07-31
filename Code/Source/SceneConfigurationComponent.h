/* Copyright 2024, Robotec.ai sp. z o.o.
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

#include <RGL/SceneConfiguration.h>
#include <AzCore/Component/Component.h>

namespace RGL
{
    class SceneConfigurationComponent : public AZ::Component
    {
    public:
        AZ_COMPONENT(SceneConfigurationComponent, "{18c28e89-27f0-4230-936e-d7858b39d53f}", AZ::Component);

        SceneConfigurationComponent() = default;
        ~SceneConfigurationComponent() override = default;

        static void Reflect(AZ::ReflectContext* context);

        // clang-format off
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType &required) {}
        // clang-format on

        // AZ::Component overrides
        void Activate() override;
        void Deactivate() override;

    private:
        SceneConfiguration m_config;
    };
} // namespace RGL
