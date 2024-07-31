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
#include <AzFramework/Physics/HeightfieldProviderBus.h>
#include <AzFramework/Terrain/TerrainDataRequestBus.h>
#include <Entity/Terrain/TerrainData.h>
#include <Utilities/RGLUtils.h>

namespace RGL
{
    bool TerrainData::UpdateBounds(const AZ::Aabb& newWorldBounds)
    {
        if (newWorldBounds == m_currentWorldBounds)
        {
            // This is only for world bound creation, not for update.
            return false;
        }

        size_t heightfieldGridColumns{}, heightfieldGridRows{};
        Physics::HeightfieldProviderRequestsBus::BroadcastResult(
            heightfieldGridColumns, &Physics::HeightfieldProviderRequests::GetHeightfieldGridColumns);
        Physics::HeightfieldProviderRequestsBus::BroadcastResult(
            heightfieldGridRows, &Physics::HeightfieldProviderRequests::GetHeightfieldGridRows);

        if (heightfieldGridColumns < 2 || heightfieldGridRows < 2)
        {
            return false;
        }

        AZ::Vector2 heightfieldGridSpacing{};
        Physics::HeightfieldProviderRequestsBus::BroadcastResult(
            heightfieldGridSpacing, &Physics::HeightfieldProviderRequests::GetHeightfieldGridSpacing);

        m_gridColumns = heightfieldGridColumns;
        m_gridRows = heightfieldGridRows;


        m_currentWorldBounds = newWorldBounds;

        m_vertices.clear();

        const size_t vertexCount = m_gridColumns * m_gridRows;
        m_vertices.reserve(vertexCount);

        const AZ::Vector3 worldMin = newWorldBounds.GetMin();
        for (size_t vertexIndexX = 0LU; vertexIndexX < m_gridColumns; ++vertexIndexX)
        {
            for (size_t vertexIndexY = 0LU; vertexIndexY < m_gridRows; ++vertexIndexY)
            {
                m_vertices.emplace_back(rgl_vec3f{
                    worldMin.GetX() + aznumeric_cast<float>(vertexIndexX) * heightfieldGridSpacing.GetX(),
                    worldMin.GetY() + aznumeric_cast<float>(vertexIndexY) * heightfieldGridSpacing.GetY(),
                    0.0f,
                });
            }
        }

        UpdateUvs();

        m_indices.clear();
        m_indices.reserve((m_gridColumns - 1) * (m_gridRows - 1) * TrianglesPerSector);
        for (size_t sectorIndexX = 0LU; sectorIndexX < m_gridColumns - 1; ++sectorIndexX)
        {
            for (size_t sectorIndexY = 0LU; sectorIndexY < m_gridRows - 1; ++sectorIndexY)
            {
                const auto lowerLeft = aznumeric_cast<int32_t>(sectorIndexY + sectorIndexX * m_gridRows);
                const auto lowerRight = aznumeric_cast<int32_t>(lowerLeft + m_gridRows);
                const auto upperLeft = aznumeric_cast<int32_t>(lowerLeft + 1);
                const auto upperRight = aznumeric_cast<int32_t>(lowerRight + 1);

                m_indices.emplace_back(rgl_vec3i{ upperLeft, lowerRight, upperRight });
                m_indices.emplace_back(rgl_vec3i{ upperLeft, lowerLeft, lowerRight });
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
        m_gridColumns = m_gridRows = 0U;
        m_vertices.clear();
        m_indices.clear();
        m_uvs.clear();
    }

    void TerrainData::SetIsTiled(bool isTiled)
    {
        if (m_isTiled == isTiled)
        {
            return;
        }

        m_isTiled = isTiled;
        UpdateUvs();
    }

    const AZStd::vector<rgl_vec3f>& TerrainData::GetVertices() const
    {
        return m_vertices;
    }

    const AZStd::vector<rgl_vec3i>& TerrainData::GetIndices() const
    {
        return m_indices;
    }

    const AZStd::vector<rgl_vec2f>& TerrainData::GetUvs() const
    {
        return m_uvs;
    }

    void TerrainData::UpdateUvs()
    {
        m_uvs.clear();
        m_uvs.reserve(m_gridColumns * m_gridRows);

        for (size_t x = 0; x < m_gridColumns; ++x)
        {
            for (size_t y = 0; y < m_gridRows; ++y)
            {
                auto uv = rgl_vec2f{
                    aznumeric_cast<float>(x),
                    aznumeric_cast<float>(y),
                };

                // Terrain Macro Material's image
                // is inverted on the v axis.
                if (!m_isTiled)
                {
                    uv.value[0] /= aznumeric_cast<float>(m_gridColumns);
                    uv.value[1] /= aznumeric_cast<float>(m_gridRows);
                    uv.value[1] = 1.0f - uv.value[1];
                }

                m_uvs.emplace_back(uv);
            }
        }
    }
} // namespace RGL
