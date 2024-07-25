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
#include <AzCore/Asset/AssetSerializer.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <RGL/RGLBus.h>
#include <SceneConfigurationComponent.h>

namespace RGL
{
    void SceneConfigurationComponent::Reflect(AZ::ReflectContext* context)
    {
        SceneConfiguration::Reflect(context);

        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<SceneConfigurationComponent, AZ::Component>()->Version(0)->Field(
                "Config", &SceneConfigurationComponent::m_config);

            if (auto* editContext = serializeContext->GetEditContext())
            {
                // clang-format off
                editContext->Class<SceneConfigurationComponent>("RGL Scene Configuration", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::Category, "RGL")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZStd::vector<AZ::Crc32>({ AZ_CRC_CE("Level") }))
                    ->DataElement(AZ::Edit::UIHandlers::Default, &SceneConfigurationComponent::m_config, "Config", "");
                // clang-format on
            }
        }
    }

    void SceneConfigurationComponent::Activate()
    {
        m_config.m_terrainIntensityConfig.m_colorImageAsset.QueueLoad();
        RGLInterface::Get()->SetSceneConfiguration(m_config);
    }

    void SceneConfigurationComponent::Deactivate()
    {
        m_config.m_terrainIntensityConfig.m_colorImageAsset.Release();
    }
} // namespace RGL
