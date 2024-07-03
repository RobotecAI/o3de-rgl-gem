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
#include <AzCore/Serialization/SerializeContext.h>
#include <Entity/Terrain/TerrainEntityManagerEditorSystemComponent.h>

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

    void TerrainEntityManagerEditorSystemComponent::Activate()
    {
        TerrainEntityManagerSystemComponent::Activate();
    }

    void TerrainEntityManagerEditorSystemComponent::Deactivate()
    {
        TerrainEntityManagerSystemComponent::Deactivate();
    }
} // namespace RGL
