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
#include <RGL/RGLBus.h>
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
        m_packedRglEntityId = Utils::PackRglEntityId(ROS2::SegmentationIds{ Utils::GenerateSegmentationEntityId(), ROS2::TerrainClassId });
        AzFramework::Terrain::TerrainDataNotificationBus::Handler::BusConnect();
        RGLNotificationBus::Handler::BusConnect();
    }

    void TerrainEntityManagerSystemComponent::Deactivate()
    {
        RGLNotificationBus::Handler::BusDisconnect();
        AzFramework::Terrain::TerrainDataNotificationBus::Handler::BusDisconnect();
        m_terrainData.Clear();
        EnsureRGLEntityDestroyed();
    }

    Wrappers::RglTexture TerrainEntityManagerSystemComponent::CreateTextureFromConfig(const TerrainIntensityConfiguration& intensityConfig)
    {
        Wrappers::RglTexture terrainTexture = Wrappers::RglTexture::CreateInvalid();

        if (intensityConfig.m_colorImageAsset.IsReady())
        {
            terrainTexture = Wrappers::RglTexture::CreateFromImageAsset(intensityConfig.m_colorImageAsset.GetId());
            if (terrainTexture.IsValid())
            {
                return terrainTexture;
            }
        }

        return { &intensityConfig.m_defaultValue, 1U, 1U };
    }

    void TerrainEntityManagerSystemComponent::OnSceneConfigurationSet(const SceneConfiguration& config)
    {
        const auto& intensityConfig = RGLInterface::Get()->GetSceneConfiguration().m_terrainIntensityConfig;
        m_rglTexture = AZStd::move(CreateTextureFromConfig(intensityConfig));
        if (m_rglTexture.IsValid() && m_rglEntity.IsValid())
        {
            m_rglEntity.SetIntensityTexture(m_rglTexture);
        }

        m_terrainData.SetIsTiled(intensityConfig.m_isTiled);
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
        m_rglEntity = AZStd::move(Wrappers::RglEntity::CreateInvalid());
        m_rglMesh = AZStd::move(Wrappers::RglMesh::CreateInvalid());
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

        const auto& vertices = m_terrainData.GetVertices();
        const auto& indices = m_terrainData.GetIndices();

        m_rglMesh = AZStd::move(Wrappers::RglMesh(vertices.data(), vertices.size(), indices.data(), indices.size()));
        if (!m_rglMesh.IsValid())
        {
            AZ_Assert(false, "The TerrainEntityManager was unable to create an RGL mesh.");
            return;
        }

        const auto& uvs = m_terrainData.GetUvs();
        m_rglMesh.SetTextureCoordinates(uvs.data(), uvs.size());

        m_rglEntity = AZStd::move(Wrappers::RglEntity(m_rglMesh));
        if (!m_rglEntity.IsValid())
        {
            AZ_Assert(false, "The TerrainEntityManager was unable to create an RGL entity.");
            return;
        }

        m_rglEntity.SetId(m_packedRglEntityId);
        m_rglEntity.SetPose(Utils::IdentityTransform);
        if (m_rglTexture.IsValid())
        {
            m_rglEntity.SetIntensityTexture(m_rglTexture);
        }
    }

    void TerrainEntityManagerSystemComponent::UpdateDirtyRegion(const AZ::Aabb& dirtyRegion)
    {
        const auto& vertices = m_terrainData.GetVertices();
        if (vertices.empty())
        {
            // Dirty regions can only be updated after data creation and before destruction. (See TerrainDataRequests).
            return;
        }

        m_terrainData.UpdateDirtyRegion(dirtyRegion);
        m_rglMesh.UpdateVertices(vertices.data(), vertices.size());
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
