/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "MeshLibrary.h"
#include "rgl/api/experimental.h"

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
            for (auto mesh : mapEntry.second)
            {
                ErrorCheck(rgl_mesh_destroy(mesh));
            }
        }
    }

    AZStd::vector<Mesh*> MeshLibrary::GetMeshPointers(const AZ::Data::Asset<AZ::RPI::ModelAsset>& modelAsset)
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
            ErrorCheck(rgl_mesh_create(&meshPointer, vertices.data(), vertices.size(), indices.data(), indices.size()));
            meshPointers.emplace_back(meshPointer);
        }

        m_meshPointersMap.insert({assetId, meshPointers});
        return meshPointers;
    }
} // namespace RGL
