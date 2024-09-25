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
#include <Wrappers/RglEntity.h>
#include <Wrappers/RglMesh.h>
#include <Wrappers/RglTexture.h>

namespace RGL
{
    MeshEntityManager::MeshEntityManager(AZ::EntityId entityId)
        : MaterialEntityManager{ entityId }
    {
        AZ::EntityBus::Handler::BusConnect(m_entityId);
    }

    MeshEntityManager::~MeshEntityManager()
    {
        AZ::EntityBus::Handler::BusDisconnect();
    }

    void MeshEntityManager::OnEntityActivated(const AZ::EntityId& entityId)
    {
        MaterialEntityManager::OnEntityActivated(entityId);
        AZ::Render::MeshComponentNotificationBus::Handler::BusConnect(entityId);
    }

    void MeshEntityManager::OnEntityDeactivated(const AZ::EntityId& entityId)
    {
        AZ::Render::MaterialComponentNotificationBus::Handler::BusDisconnect();
        AZ::Render::MeshComponentNotificationBus::Handler::BusDisconnect();
        MaterialEntityManager::OnEntityDeactivated(entityId);
    }

    void MeshEntityManager::OnModelReady(
        const AZ::Data::Asset<AZ::RPI::ModelAsset>& modelAsset, [[maybe_unused]] const AZ::Data::Instance<AZ::RPI::Model>& model)
    {
        AZ_Assert(m_entities.empty(), "Entity Manager for entity with ID: %s has an invalid state.", m_entityId.ToString().c_str());
        auto* modelLibrary = ModelLibraryInterface::Get();
        const MeshMaterialSlotPairList& meshes = modelLibrary->StoreModelAsset(modelAsset);

        if (meshes.empty())
        {
            AZ_Assert(false, "MeshEntityManager with ID: %s did not receive any mesh from the ModelLibrary.", m_entityId.ToString().c_str());
            return;
        }

        m_entities.reserve(meshes.size());
        size_t entityIdx = 0;
        for (const auto& [mesh, matSlot] : meshes)
        {
            Wrappers::RglEntity entity(mesh);
            if (entity.IsValid())
            {
                AssignMaterialSlotIdForMesh(matSlot.m_stableId, entityIdx);
                const Wrappers::RglTexture& texture = modelLibrary->StoreMaterialAsset(matSlot.m_defaultMaterialAsset);
                if (texture.IsValid())
                {
                    entity.SetIntensityTexture(texture);
                }

                m_entities.emplace_back(AZStd::move(entity));
                ++entityIdx;
            }
        }

        m_isPoseUpdateNeeded = true;

        // We can use material info only when the model is ready.
        AZ::Render::MaterialComponentNotificationBus::Handler::BusConnect(m_entityId);
    }

    void MeshEntityManager::OnModelPreDestroy()
    {
        AZ::Render::MaterialComponentNotificationBus::Handler::BusDisconnect();
        ResetMaterialsMapping();
        m_entities.clear();
    }
} // namespace RGL
