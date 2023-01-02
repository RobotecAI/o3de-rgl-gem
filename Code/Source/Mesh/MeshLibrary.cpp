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
#include "MeshLibrary.h"

#include "rgl/api/core.h"

namespace RGL
{
    MeshLibrary::MeshLibrary()
    {
        if (MeshLibraryInterface::Get() == nullptr)
        {
            MeshLibraryInterface::Register(this);
        }

        MeshLibraryRequestBus::Handler::BusConnect();
    }

    MeshLibrary::~MeshLibrary()
    {
        if (MeshLibraryInterface::Get() == this)
        {
            MeshLibraryInterface::Unregister(this);
        }

        MeshLibraryRequestBus::Handler::BusDisconnect();

        for (auto mapEntry : m_meshPointersMap)
        {
            for (rgl_mesh_t mesh : mapEntry.second)
            {
                RglUtils::ErrorCheck(rgl_mesh_destroy(mesh));
            }
        }
    }

    void MeshLibrary::Clear()
    {
        m_meshPointersMap.clear();
    }

    AZStd::vector<rgl_mesh_t> MeshLibrary::GetMeshPointers(const AZ::Data::Asset<AZ::RPI::ModelAsset>& modelAsset)
    {
        const AZ::Data::AssetId& assetId = modelAsset.GetId();

        auto meshPointersIt = m_meshPointersMap.find(assetId);
        if (meshPointersIt != m_meshPointersMap.end())
        {
            return meshPointersIt->second;
        }

        auto lodAssets = modelAsset->GetLodAssets();
        // Get Highest LOD
        auto modelLodAsset = lodAssets.begin()->Get();
        auto meshes = modelLodAsset->GetMeshes();

        AZStd::vector<rgl_mesh_t> meshPointers;
        meshPointers.reserve(meshes.size());
        for (auto mesh : meshes)
        {
            AZStd::span<const rgl_vec3f> vertices = mesh.GetSemanticBufferTyped<rgl_vec3f>(AZ::Name("POSITION"));
            AZStd::span<const rgl_vec3i> indices = mesh.GetIndexBufferTyped<rgl_vec3i>();

            rgl_mesh_t meshPointer = nullptr;
            RglUtils::SafeRglMeshCreate(meshPointer, vertices.data(), vertices.size(), indices.data(), indices.size());
            if (meshPointer == nullptr)
            {
                continue;
            }

            meshPointers.emplace_back(meshPointer);
        }

        m_meshPointersMap.insert({ assetId, meshPointers });
        return meshPointers;
    }
} // namespace RGL
