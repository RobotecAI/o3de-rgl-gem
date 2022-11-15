/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

#include "MeshLibraryBus.h"
#include <AtomLyIntegration/CommonFeatures/Mesh/MeshComponentBus.h>
#include <AzCore/Asset/AssetCommon.h>
#include <AzCore/std/containers/unordered_map.h>

namespace RGL
{
    //! Class providing easy access to RGL's meshes.
    //! Each mesh has a corresponding modelAsset by which it is accessed.
    class MeshLibrary : protected MeshLibraryRequestBus::Handler
    {
    public:
        MeshLibrary();
        MeshLibrary(MeshLibrary&& meshLibrary) = delete;
        MeshLibrary(const MeshLibrary& meshLibrary) = delete;
        ~MeshLibrary();

    protected:
        ////////////////////////////////////////////////////////////////////////
        // MeshLibraryRequestBus::Handler implementation
        AZStd::vector<Mesh*> GetMeshPointers(const AZ::Data::Asset<AZ::RPI::ModelAsset>& modelAsset) override;
        ////////////////////////////////////////////////////////////////////////
    private:
        AZStd::unordered_map<AZ::Data::AssetId, AZStd::vector<Mesh*>> m_meshPointersMap;
    };
} // namespace RGL