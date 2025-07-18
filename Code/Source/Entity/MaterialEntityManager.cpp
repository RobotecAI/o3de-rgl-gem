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
#include <AtomLyIntegration/CommonFeatures/Material/MaterialComponentConstants.h>
#include <Entity/MaterialEntityManager.h>
#include <Model/ModelLibraryBus.h>
#include <Wrappers/RglTexture.h>

namespace RGL
{
    MaterialEntityManager::MaterialEntityManager(AZ::EntityId entityId)
        : EntityManager(entityId)
    {
    }

    void MaterialEntityManager::OnMaterialsUpdated(const AZ::Render::MaterialAssignmentMap& materials)
    {
        if (m_entities.empty())
        {
            AZ_Warning(__func__, false, "Skipping material update. The entities were not yet created.");
            return;
        }

        for (const auto& [assignmentId, assignment] : materials)
        {
            const Wrappers::RglTexture& materialTexture = ModelLibraryInterface::Get()->StoreMaterialAsset(assignment.m_materialAsset);
            if (materialTexture.IsValid())
            {
                if (assignmentId.m_materialSlotStableId == AZ::RPI::ModelMaterialSlot::InvalidStableId)
                {
                    AZStd::string entityName;
                    AZ::ComponentApplicationBus::BroadcastResult(entityName, &AZ::ComponentApplicationRequests::GetEntityName, m_entityId);
                    AZ_Warning(
                        __func__,
                        false,
                        "MaterialEntityManager::OnMaterialsUpdated: Invalid stable ID listed in list of updated materials in entity %s.",
                        entityName.c_str());
                    continue;
                }

                size_t meshEntityIdx = GetMeshEntityIdxForMaterialSlotId(assignmentId.m_materialSlotStableId);
                if (meshEntityIdx == -1)
                {
                    continue;
                }

                m_entities[meshEntityIdx].SetIntensityTexture(materialTexture);
            }
        }
    }

    void MaterialEntityManager::AssignMaterialSlotIdForMesh(AZ::RPI::ModelMaterialSlot::StableId materialSlotId, size_t meshEntityIdx)
    {
        m_materialSlotMeshIdMap.emplace(materialSlotId, meshEntityIdx);
    }

    void MaterialEntityManager::ResetMaterialsMapping()
    {
        m_materialSlotMeshIdMap.clear();
    }

    size_t MaterialEntityManager::GetMeshEntityIdxForMaterialSlotId(AZ::RPI::ModelMaterialSlot::StableId materialSlotId) const
    {
        auto it = m_materialSlotMeshIdMap.find(materialSlotId);
        if (it == m_materialSlotMeshIdMap.end())
        {
            AZStd::string entityName;
            AZ::ComponentApplicationBus::BroadcastResult(entityName, &AZ::ComponentApplicationRequests::GetEntityName, m_entityId);
            AZ_Error(
                __func__,
                false,
                "Programmer error: Unable to find mesh entity associated with provided material slot id: %u in entity %s.",
                materialSlotId,
                entityName.c_str());
            return -1;
        }

        return it->second;
    }
} // namespace RGL
