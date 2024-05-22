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

#include <Entity/MeshEntityManager.h>
#include <Mesh/MeshLibraryBus.h>
#include <Utilities/RGLUtils.h>

namespace RGL
{
    MeshEntityManager::MeshEntityManager(AZ::EntityId entityId)
        : EntityManager{ entityId }
    {
        AZ::Render::MeshComponentNotificationBus::Handler::BusConnect(entityId);
    }

    MeshEntityManager::~MeshEntityManager()
    {
        AZ::Render::MeshComponentNotificationBus::Handler::BusDisconnect();
    }

    void MeshEntityManager::OnModelReady(
        const AZ::Data::Asset<AZ::RPI::ModelAsset>& modelAsset, [[maybe_unused]] const AZ::Data::Instance<AZ::RPI::Model>& model)
    {
        AZ_Assert(m_entities.empty(), "Entity Manager for entity with ID: %s has an invalid state.", m_entityId.ToString().c_str());
        const auto meshes = MeshLibraryInterface::Get()->StoreModelAsset(modelAsset);

        if (meshes.empty())
        {
            AZ_Assert(false, "MeshEntityManager with ID: %s did not receive any mesh from the MeshLibrary.", m_entityId.ToString().c_str());
            return;
        }

        // TODO: Segment counter is a work-in-progress entity id substitute. Target solution depends on the requirements, e.g.
        //  - if we want to set id manually, we probably need dedicated component.
        //  - if we want to set id based on assets, we need a way to serialize them (their names? e.g. static hash map with asset names and
        //      id from static counter - different instances should have the same id).
        // TODO: May also be worth to double check if OnModelReady may not be called asynchronously. However, this probably would have risen
        // problems earlier in RGL.
        static int32_t segmentCounter = 1;
        m_entities.reserve(meshes.size());

        for (rgl_mesh_t mesh : meshes)
        {
            rgl_entity_t entity = nullptr;
            Utils::SafeRglEntityCreate(entity, mesh, segmentCounter);
            if (entity)
            {
                m_entities.emplace_back(entity);
            }
        }

        ++segmentCounter;
        m_isPoseUpdateNeeded = true;
    }
} // namespace RGL