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
#include <Mesh/MeshLibrary.h>
#include <Utilities/RGLUtils.h>
#include <rgl/api/core.h>

namespace RGL
{
    MeshLibrary::MeshLibrary()
    {
        if (!MeshLibraryInterface::Get())
        {
            MeshLibraryInterface::Register(this);
        }

        MeshLibraryRequestBus::Handler::BusConnect();
    }

    MeshLibrary::MeshLibrary(MeshLibrary&& meshLibrary)
        : m_modelMeshesMap{ AZStd::move(meshLibrary.m_modelMeshesMap) }
    {
        meshLibrary.BusDisconnect();
        MeshLibraryInterface::Unregister(&meshLibrary);
        MeshLibraryInterface::Register(this);
        MeshLibraryRequestBus::Handler::BusConnect();
    }

    MeshLibrary::~MeshLibrary()
    {
        if (MeshLibraryInterface::Get() == this)
        {
            MeshLibraryInterface::Unregister(this);
        }

        MeshLibraryRequestBus::Handler::BusDisconnect();

        Clear();
    }

    void MeshLibrary::Clear()
    {
        m_modelMeshesMap.clear();
    }

    const MeshList& MeshLibrary::StoreModelAsset(const AZ::Data::Asset<AZ::RPI::ModelAsset>& modelAsset)
    {
        const AZ::Data::AssetId& assetId = modelAsset.GetId();

        if (auto meshPointersIt = m_modelMeshesMap.find(assetId); meshPointersIt != m_modelMeshesMap.end())
        {
            return meshPointersIt->second;
        }

        const auto lodAssets = modelAsset->GetLodAssets();
        // Get Highest LOD
        const auto modelLodAsset = lodAssets.begin()->Get();
        const auto meshes = modelLodAsset->GetMeshes();

        MeshList modelMeshes;
        modelMeshes.reserve(meshes.size());
        for (auto& mesh : meshes)
        {
            const AZStd::span<const rgl_vec3f> vertices = mesh.GetSemanticBufferTyped<rgl_vec3f>(AZ::Name("POSITION"));
            const AZStd::span<const rgl_vec3i> indices = mesh.GetIndexBufferTyped<rgl_vec3i>();

            Wrappers::Mesh rglMesh(vertices.data(), vertices.size(), indices.data(), indices.size());
            if (!rglMesh.IsValid())
            {
                continue;
            }

            const AZStd::span<const rgl_vec2f> uvs = mesh.GetSemanticBufferTyped<rgl_vec2f>(AZ::Name("UV"));
            rglMesh.SetTextureCoordinates(uvs.data(), uvs.size());

            modelMeshes.emplace_back(AZStd::move(rglMesh));
        }

        const auto entryIt = m_modelMeshesMap.emplace(assetId, AZStd::move(modelMeshes));
        return entryIt.first->second;
    }
} // namespace RGL
