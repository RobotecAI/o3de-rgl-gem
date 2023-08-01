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
#pragma once

#include <AtomLyIntegration/CommonFeatures/Mesh/MeshComponentBus.h>
#include <AzCore/Asset/AssetCommon.h>
#include <AzCore/std/containers/unordered_map.h>
#include <Mesh/MeshLibraryBus.h>
#include <rgl/api/core.h>

namespace RGL
{
    //! Class providing easy access to RGL's meshes.
    //! Each mesh has a corresponding modelAsset by which it is accessed.
    class MeshLibrary : protected MeshLibraryRequestBus::Handler
    {
    public:
        MeshLibrary();
        MeshLibrary(MeshLibrary&& meshLibrary);
        MeshLibrary(const MeshLibrary& meshLibrary) = default;
        ~MeshLibrary();

        //! Deletes all meshes stored by the Library.
        void Clear();

    protected:
        // MeshLibraryRequestBus overrides
        AZStd::vector<rgl_mesh_t> StoreModelAsset(const AZ::Data::Asset<AZ::RPI::ModelAsset>& modelAsset) override;

    private:
        AZStd::unordered_map<AZ::Data::AssetId, AZStd::vector<Mesh*>> m_meshPointersMap;
    };
} // namespace RGL