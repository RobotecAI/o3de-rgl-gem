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

#include <AzCore/Asset/AssetCommon.h>
#include <AzCore/std/containers/unordered_map.h>
#include <Model/ModelLibraryBus.h>
#include <Wrappers/RglMesh.h>
#include <Wrappers/RglTexture.h>
#include <rgl/api/core.h>

namespace RGL
{
    //! Class providing easy access to RGL's meshes.
    //! Each mesh has a corresponding modelAsset by which it is accessed.
    class ModelLibrary : protected ModelLibraryRequestBus::Handler
    {
    public:
        ModelLibrary();
        ModelLibrary(ModelLibrary&& modelLibrary);
        ModelLibrary(const ModelLibrary& modelLibrary) = delete;
        ~ModelLibrary();

        //! Deletes all meshes and textures stored by the Library.
        void Clear();

    protected:
        // ModelLibraryRequestBus overrides
        const MeshMaterialSlotPairList& StoreModelAsset(const AZ::Data::Asset<AZ::RPI::ModelAsset>& modelAsset) override;
        const Wrappers::RglTexture& StoreMaterialAsset(const AZ::Data::Asset<AZ::RPI::MaterialAsset>& materialAsset) override;
        Wrappers::RglTexture m_invalidTexture{ AZStd::move(Wrappers::RglTexture::CreateInvalid()) };

    private:
        using MeshMap = AZStd::unordered_map<AZ::Data::AssetId, MeshMaterialSlotPairList>;
        using TextureMap = AZStd::unordered_map<AZ::Data::AssetId, Wrappers::RglTexture>;

        MeshMap m_meshMap;
        TextureMap m_textureMap;
    };
} // namespace RGL