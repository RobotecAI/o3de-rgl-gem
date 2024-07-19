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
    void TerrainIntensityConfiguration::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<TerrainIntensityConfiguration>()
                ->Version(0)
                ->Field("DefaultIntensity", &TerrainIntensityConfiguration::m_defaultValue)
                ->Field("ColorTexture", &TerrainIntensityConfiguration::m_colorImageAsset)
                ->Field("Tiled", &TerrainIntensityConfiguration::m_isTiled);

            if (auto* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<TerrainIntensityConfiguration>("RGL Intensity Configuration", "")
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default,
                        &TerrainIntensityConfiguration::m_defaultValue,
                        "Default Intensity",
                        "Defines a default intensity value to be used when no color texture is provided.")
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default,
                        &TerrainIntensityConfiguration::m_colorImageAsset,
                        "Color Texture",
                        "Color texture is used to calculate intensity. This texture is not required.")
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default,
                        &TerrainIntensityConfiguration::m_isTiled,
                        "Tiled",
                        "If enabled, the provided color texture is tiled over the terrain grid. Enabled by default");
            }
        }
    }

    void SceneConfiguration::Reflect(AZ::ReflectContext* context)
    {
        TerrainIntensityConfiguration::Reflect(context);

        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<SceneConfiguration>()
                ->Version(0)
                ->Field("TerrainIntensityConfig", &SceneConfiguration::m_terrainIntensityConfig)
                ->Field("SkinnedMeshUpdate", &SceneConfiguration::m_isSkinnedMeshUpdateEnabled);

            if (auto* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<SceneConfiguration>("RGL Scene Configuration", "")
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &SceneConfiguration::m_terrainIntensityConfig, "Terrain Intensity Configuration", "")
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default,
                        &SceneConfiguration::m_isSkinnedMeshUpdateEnabled,
                        "Skinned Mesh Update",
                        "Should the Skinned Meshes be updated?");
            }
        }
    }

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
