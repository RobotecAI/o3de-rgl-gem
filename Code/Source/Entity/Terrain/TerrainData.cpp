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
#include <Entity/Terrain/TerrainData.h>

#include <AzFramework/Physics/HeightfieldProviderBus.h>
#include <AzFramework/Terrain/TerrainDataRequestBus.h>

namespace RGL
{
    bool TerrainData::UpdateBounds(AZ::Aabb newWorldBounds)
    {
        if (newWorldBounds == m_currentWorldBounds)
        {
            // This is only for world bound creation, not for update.
            return false;
        }

        m_currentWorldBounds = newWorldBounds;

        size_t heightfieldGridColumns{}, heighfieldGridRows{};
        Physics::HeightfieldProviderRequestsBus::BroadcastResult(
            heightfieldGridColumns, &Physics::HeightfieldProviderRequests::GetHeightfieldGridColumns);
        Physics::HeightfieldProviderRequestsBus::BroadcastResult(
            heighfieldGridRows, &Physics::HeightfieldProviderRequests::GetHeightfieldGridRows);

        if (heightfieldGridColumns < 2 || heighfieldGridRows < 2)
        {
            return false;
        }

        AZ::Vector2 heightfieldGridSpacing{};
        Physics::HeightfieldProviderRequestsBus::BroadcastResult(
            heightfieldGridSpacing, &Physics::HeightfieldProviderRequests::GetHeightfieldGridSpacing);

        m_vertices.clear();
        AZ_Printf(
            "TerrainEntityManagerSystemComponent",
            "Creating terrain mesh with %zu columns and %zu rows.",
            heightfieldGridColumns,
            heighfieldGridRows);
        m_vertices.reserve(heightfieldGridColumns * heighfieldGridRows);
        const AZ::Vector3 worldMin = m_currentWorldBounds.GetMin();

        for (size_t vertexIndexX = 0LU; vertexIndexX < heightfieldGridColumns; ++vertexIndexX)
        {
            for (size_t vertexIndexY = 0LU; vertexIndexY < heighfieldGridRows; ++vertexIndexY)
            {
                m_vertices.emplace_back(rgl_vec3f{
                    worldMin.GetX() + aznumeric_cast<float>(vertexIndexX) * heightfieldGridSpacing.GetX(),
                    worldMin.GetY() + aznumeric_cast<float>(vertexIndexY) * heightfieldGridSpacing.GetY(),
                    0.0f,
                });
            }
        }

        m_indices.clear();
        m_indices.reserve((heightfieldGridColumns - 1) * (heighfieldGridRows - 1) * TrianglesPerSector);
        for (size_t sectorIndexX = 0LU; sectorIndexX < heightfieldGridColumns - 1; ++sectorIndexX)
        {
            for (size_t sectorIndexY = 0LU; sectorIndexY < heighfieldGridRows - 1; ++sectorIndexY)
            {
                const auto lowerLeft = aznumeric_cast<int32_t>(sectorIndexY + sectorIndexX * heighfieldGridRows);
                const auto lowerRight = aznumeric_cast<int32_t>(lowerLeft + heighfieldGridRows);
                const auto upperLeft = aznumeric_cast<int32_t>(lowerLeft + 1);
                const auto upperRight = aznumeric_cast<int32_t>(lowerRight + 1);

                m_indices.emplace_back(rgl_vec3i{ upperLeft, upperRight, lowerLeft });
                m_indices.emplace_back(rgl_vec3i{ lowerLeft, upperRight, lowerRight });
            }
        }

        return true;
    }

    void TerrainData::UpdateDirtyRegion(const AZ::Aabb& dirtyRegion)
    {
        const AZ::Aabb dirtyRegion2D = AZ::Aabb::CreateFromMinMaxValues(
            dirtyRegion.GetMin().GetX(),
            dirtyRegion.GetMin().GetY(),
            AZStd::numeric_limits<float>::lowest(),
            dirtyRegion.GetMax().GetX(),
            dirtyRegion.GetMax().GetY(),
            AZStd::numeric_limits<float>::max());

        for (rgl_vec3f& vertex : m_vertices)
        {
            if (dirtyRegion2D.Contains(Utils::AzVector3FromRglVec3f(vertex)))
            {
                float height = AZStd::numeric_limits<float>::lowest();
                bool terrainExists = false;

                // Sampler::Exact is used because mesh vertices are created directly on grid provided from heightfield.
                AzFramework::Terrain::TerrainDataRequestBus::BroadcastResult(
                    height,
                    &AzFramework::Terrain::TerrainDataRequestBus::Events::GetHeightFromFloats,
                    vertex.value[0],
                    vertex.value[1],
                    AzFramework::Terrain::TerrainDataRequests::Sampler::EXACT,
                    &terrainExists);

                if (height != AZStd::numeric_limits<float>::lowest() && terrainExists)
                {
                    vertex.value[2] = height;
                }
            }
        }
    }

    void TerrainData::Clear()
    {
        m_currentWorldBounds = AZ::Aabb::CreateFromPoint(AZ::Vector3::CreateZero());
        m_vertices.clear();
        m_indices.clear();
    }
} // namespace RGL
