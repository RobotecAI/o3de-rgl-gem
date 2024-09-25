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
#pragma once

#include <AtomLyIntegration/CommonFeatures/Material/MaterialComponentBus.h>
#include <AzCore/std/containers/unordered_map.h>
#include <Entity/EntityManager.h>

namespace RGL
{
    //! Base class used for managing RGL's representation of an Entity with a Material component.
    //! Although it already implements the EntityBus and MaterialComponentNotificationBus handlers,
    //! the derived classes have to handle bus connections
    //! through BusConnect and BusDisconnect function calls.
    //! This is to allow for further overrides.
    class MaterialEntityManager
        : public EntityManager
        , protected AZ::Render::MaterialComponentNotificationBus::Handler
    {
    public:
        explicit MaterialEntityManager(AZ::EntityId entityId);
        MaterialEntityManager(const MaterialEntityManager& other) = delete;
        MaterialEntityManager(MaterialEntityManager&& other) = delete;
        MaterialEntityManager& operator=(MaterialEntityManager&& rhs) = delete;
        MaterialEntityManager& operator=(const MaterialEntityManager&) = delete;
        ~MaterialEntityManager() = default;

    protected:
        size_t GetMeshEntityIdxForMaterialSlotId(AZ::RPI::ModelMaterialSlot::StableId materialSlotId) const;

        void AssignMaterialSlotIdForMesh(AZ::RPI::ModelMaterialSlot::StableId materialSlotId, size_t meshEntityIdx);
        void ResetMaterialsMapping();

    private:
        // AZ::Render::MaterialComponentNotificationBus implementation overrides
        void OnMaterialsUpdated(const AZ::Render::MaterialAssignmentMap& materials) override;

        AZStd::unordered_map<AZ::RPI::ModelMaterialSlot::StableId, size_t> m_materialSlotMeshIdMap;
    };
} // namespace RGL
