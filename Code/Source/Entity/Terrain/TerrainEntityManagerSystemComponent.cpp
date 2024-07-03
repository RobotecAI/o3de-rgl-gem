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
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <Entity/Terrain/TerrainEntityManagerSystemComponent.h>
#include <Utilities/RGLUtils.h>

namespace RGL
{
    void TerrainEntityManagerSystemComponent::OnTerrainDataCreateEnd()
    {
        UpdateWorldBounds();
    }

    void TerrainEntityManagerSystemComponent::OnTerrainDataDestroyEnd()
    {
        m_terrainData.Clear();
        EnsureRGLEntityDestroyed();
    }

    void TerrainEntityManagerSystemComponent::Activate()
    {
        AzFramework::Terrain::TerrainDataNotificationBus::Handler::BusConnect();
    }

    void TerrainEntityManagerSystemComponent::Deactivate()
    {
        AzFramework::Terrain::TerrainDataNotificationBus::Handler::BusDisconnect();
        m_terrainData.Clear();
        EnsureRGLEntityDestroyed();
    }

    void TerrainEntityManagerSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<TerrainEntityManagerSystemComponent, AZ::Component>()->Version(0);

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<TerrainEntityManagerSystemComponent>("Terrain Entity Manager", "Manages the RGL Terrain entity.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
                    ->Attribute(AZ::Edit::Attributes::Category, "RGL")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true);
            }
        }
    }

    void TerrainEntityManagerSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("TerrainEntityManagerService"));
    }

    void TerrainEntityManagerSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("TerrainEntityManagerService"));
    }

    void TerrainEntityManagerSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("RGLService"));
    }

    void TerrainEntityManagerSystemComponent::EnsureRGLEntityDestroyed()
    {
        if (m_rglEntity)
        {
            RGL_CHECK(rgl_entity_destroy(m_rglEntity));
            m_rglEntity = nullptr;
        }

        if (m_rglMesh)
        {
            RGL_CHECK(rgl_mesh_destroy(m_rglMesh));
            m_rglMesh = nullptr;
        }
    }

    void TerrainEntityManagerSystemComponent::UpdateWorldBounds()
    {
        AZ::Aabb newWorldBounds = AZ::Aabb::CreateFromPoint(AZ::Vector3::CreateZero());
        AzFramework::Terrain::TerrainDataRequestBus::BroadcastResult(
            newWorldBounds, &AzFramework::Terrain::TerrainDataRequests::GetTerrainAabb);

        if (!m_terrainData.UpdateBounds(newWorldBounds))
        {
            // The terrain data was not updated. No need to update the RGL mesh.
            return;
        }

        EnsureRGLEntityDestroyed();

        Utils::SafeRglMeshCreate(
            m_rglMesh,
            m_terrainData.m_vertices.data(),
            m_terrainData.m_vertices.size(),
            m_terrainData.m_indices.data(),
            m_terrainData.m_indices.size());

        if (!m_rglMesh)
        {
            AZ_Assert(false, "The TerrainEntityManager was unable to create an RGL mesh.");
            return;
        }

        Utils::SafeRglEntityCreate(m_rglEntity, m_rglMesh);
        if (!m_rglEntity)
        {
            AZ_Assert(false, "The TerrainEntityManager was unable to create an RGL entity.");
            return;
        }

        RGL_CHECK(rgl_entity_set_pose(m_rglEntity, &Utils::IdentityTransform));
    }

    void TerrainEntityManagerSystemComponent::UpdateDirtyRegion(const AZ::Aabb& dirtyRegion)
    {
        if (m_terrainData.m_vertices.empty())
        {
            // Dirty regions can only be updated after data creation and before destruction. (See TerrainDataRequests).
            return;
        }

        m_terrainData.UpdateDirtyRegion(dirtyRegion);
        RGL_CHECK(
            rgl_mesh_update_vertices(m_rglMesh, m_terrainData.m_vertices.data(), aznumeric_cast<int32_t>(m_terrainData.m_vertices.size())));
    }

    void TerrainEntityManagerSystemComponent::OnTerrainDataChanged(const AZ::Aabb& dirtyRegion, TerrainDataChangedMask dataChangedMask)
    {
        if ((dataChangedMask & TerrainDataChangedMask::Settings) != TerrainDataChangedMask::None)
        {
            UpdateWorldBounds();
        }

        if ((dataChangedMask & TerrainDataChangedMask::HeightData) != TerrainDataChangedMask::None)
        {
            UpdateDirtyRegion(dirtyRegion);
        }
    }
} // namespace RGL