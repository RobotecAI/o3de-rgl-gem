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

#include <Entity/Terrain/TerrainEntityManagerSystemComponent.h>

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

        TerrainEntityManagerEditorSystemComponent() = default;
        ~TerrainEntityManagerEditorSystemComponent() override = default;

    private:
        void Activate() override;
        void Deactivate() override;
    };
} // namespace RGL
