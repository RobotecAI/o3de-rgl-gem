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
#include <RGL/SceneConfiguration.h>

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
                        "Defines a default intensity value to be used when no color texture is provided. Must be in range [0, 255]."
                        "Set to 0 by default.")
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

} // namespace RGL