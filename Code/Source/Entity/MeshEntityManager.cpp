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

#include <AtomLyIntegration/CommonFeatures/Material/MaterialComponentConstants.h>
#include <Entity/MeshEntityManager.h>
#include <Model/ModelLibraryBus.h>
#include <Utilities/RGLUtils.h>
#include <Wrappers/Entity.h>
#include <Wrappers/Mesh.h>
#include <Wrappers/Texture.h>

namespace RGL
{
    MeshEntityManager::MeshEntityManager(AZ::EntityId entityId)
        : EntityManager{ entityId }
    {
    }

    void MeshEntityManager::OnEntityActivated(const AZ::EntityId& entityId)
    {
        EntityManager::OnEntityActivated(entityId);
        AZ::Render::MeshComponentNotificationBus::Handler::BusConnect(entityId);

        AZ::Entity* entity = AZ::Interface<AZ::ComponentApplicationRequests>::Get()->FindEntity(entityId);
        if (entity->FindComponent(AZ::Render::MaterialComponentTypeId))
        {
            AZ::Render::MaterialComponentNotificationBus::Handler::BusConnect(entityId);
        }
    }

    void MeshEntityManager::OnEntityDeactivated(const AZ::EntityId& entityId)
    {
        AZ::Render::MaterialComponentNotificationBus::Handler::BusDisconnect();
        AZ::Render::MeshComponentNotificationBus::Handler::BusDisconnect();
        EntityManager::OnEntityDeactivated(entityId);
    }

    void MeshEntityManager::OnModelReady(
        const AZ::Data::Asset<AZ::RPI::ModelAsset>& modelAsset, [[maybe_unused]] const AZ::Data::Instance<AZ::RPI::Model>& model)
    {
        AZ_Assert(m_entities.empty(), "Entity Manager for entity with ID: %s has an invalid state.", m_entityId.ToString().c_str());
        auto* modelLibrary = ModelLibraryInterface::Get();
        const MeshMaterialSlotPairList& meshes = modelLibrary->StoreModelAsset(modelAsset);

        if (meshes.empty())
        {
            AZ_Assert(false, "MeshEntityManager with ID: %s did not receive any mesh from the MeshLibrary.", m_entityId.ToString().c_str());
            return;
        }

        m_entities.reserve(meshes.size());
        size_t entityIdx = 0;
        for (const auto& [mesh, matSlot] : meshes)
        {
            Wrappers::Entity entity(mesh);
            if (entity.IsValid())
            {
                m_materialSlotMeshIdMap.emplace(matSlot.m_stableId, entityIdx);
                const Wrappers::Texture& defaultTexture = modelLibrary->StoreMaterialAsset(matSlot.m_defaultMaterialAsset);
                entity.SetIntensityTexture(defaultTexture);
                m_entities.emplace_back(AZStd::move(entity));
                ++entityIdx;
            }
        }

        m_isPoseUpdateNeeded = true;
    }

    void MeshEntityManager::OnMaterialsUpdated(const AZ::Render::MaterialAssignmentMap& materials)
    {
        for (const auto& [assignmentId, assignment] : materials)
        {
            const Wrappers::Texture& materialAssetTexture = ModelLibraryInterface::Get()->StoreMaterialAsset(assignment.m_materialAsset);
            m_entities[m_materialSlotMeshIdMap[assignmentId.m_materialSlotStableId]].SetIntensityTexture(materialAssetTexture);
        }
    }
} // namespace RGL