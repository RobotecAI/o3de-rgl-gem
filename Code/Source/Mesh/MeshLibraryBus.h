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

#include <AzCore/Component/EntityId.h>
#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

struct Mesh;

namespace RGL
{
    class MeshLibraryRequests
    {
    public:
        AZ_RTTI(MeshLibraryRequests, "{b84ccaae-5d0f-410a-821e-5ff8d449b851}");

        //! Returns a vector of RGL meshes created from the modelAsset.
        //! If the provided modelAsset was not encountered before, created RGL meshes are stored by the library.
        //! On the other hand if the RGL meshes associated with the provided modelAsset were stored it will simply retrieve them.
        //! @param modelAsset Model asset provided for storage.
        //! @return List of RGL meshes created using the provided model asset.
        virtual AZStd::vector<Mesh*> StoreModelAsset(const AZ::Data::Asset<AZ::RPI::ModelAsset>& modelAsset) = 0;

    protected:
        ~MeshLibraryRequests() = default;
    };

    class MeshLibraryBusTraits : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using MeshLibraryRequestBus = AZ::EBus<MeshLibraryRequests, MeshLibraryBusTraits>;
    using MeshLibraryInterface = AZ::Interface<MeshLibraryRequests>;
} // namespace RGL