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
#include <Model/ModelLibrary.h>
#include <Utilities/RGLUtils.h>
#include <rgl/api/core.h>

namespace RGL
{
    ModelLibrary::ModelLibrary()
    {
        if (!ModelLibraryInterface::Get())
        {
            ModelLibraryInterface::Register(this);
        }

        ModelLibraryRequestBus::Handler::BusConnect();
    }

    ModelLibrary::ModelLibrary(ModelLibrary&& modelLibrary)
        : m_meshMap{ AZStd::move(modelLibrary.m_meshMap) }
        // , m_textureMap{ AZStd::move(modelLibrary.m_textureMap) }
    {
        modelLibrary.BusDisconnect();
        ModelLibraryInterface::Unregister(&modelLibrary);
        ModelLibraryInterface::Register(this);
        ModelLibraryRequestBus::Handler::BusConnect();
    }

    ModelLibrary::~ModelLibrary()
    {
        if (ModelLibraryInterface::Get() == this)
        {
            ModelLibraryInterface::Unregister(this);
        }

        ModelLibraryRequestBus::Handler::BusDisconnect();

        Clear();
    }

    void ModelLibrary::Clear()
    {
        m_meshMap.clear();
        m_textureMap.clear();
    }

    const MeshMaterialSlotPairList& ModelLibrary::StoreModelAsset(const AZ::Data::Asset<AZ::RPI::ModelAsset>& modelAsset)
    {
        const AZ::Data::AssetId& assetId = modelAsset.GetId();
        if (auto meshPointersIt = m_meshMap.find(assetId); meshPointersIt != m_meshMap.end())
        {
            return meshPointersIt->second;
        }

        const auto lodAssets = modelAsset->GetLodAssets();
        // Get Highest LOD
        const auto modelLodAsset = lodAssets.begin()->Get();
        const auto meshes = modelLodAsset->GetMeshes();

        MeshMaterialSlotPairList modelMeshes;
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

            const AZ::RPI::ModelMaterialSlot& slot = modelAsset->FindMaterialSlot(mesh.GetMaterialSlotId());
            modelMeshes.emplace_back(AZStd::move(rglMesh), slot);
        }

        return m_meshMap.emplace(assetId, AZStd::move(modelMeshes)).first->second;
    }

    const Wrappers::Texture& ModelLibrary::StoreMaterialAsset(const AZ::Data::Asset<AZ::RPI::MaterialAsset>& materialAsset)
    {
        const AZ::Data::AssetId& assetId = materialAsset.GetId();
        if (auto textureIt = m_textureMap.find(assetId); textureIt != m_textureMap.end())
        {
            return textureIt->second;
        }

        return m_textureMap.emplace(assetId, AZStd::move(Wrappers::Texture::CreateFromMaterialAsset(materialAsset))).first->second;
    }
} // namespace RGL
