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

    MeshEntityManager::MeshEntityManager(MeshEntityManager&& other)
        : EntityManager{ AZStd::move(other) }
    {
        AZ::Render::MeshComponentNotificationBus::Handler::BusConnect(m_entityId);
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

        m_entities.reserve(meshes.size());
        for (rgl_mesh_t mesh : meshes)
        {
            rgl_entity_t entity = nullptr;
            Utils::SafeRglEntityCreate(entity, mesh);
            if (entity)
            {
                m_entities.emplace_back(entity);
            }
        }
    }
} // namespace RGL