/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
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
        virtual ~MeshLibraryRequests() = default;

        //! Returns a vector of RGL meshes created from the modelAsset.
        //! If the provided modelAsset was not encountered before, created RGL meshes are stored by the library.
        //! On the other hand if the RGL meshes associated with the provided modelAsset were stored it will simply retrieve them.
        virtual AZStd::vector<Mesh*> GetMeshPointers(const AZ::Data::Asset<AZ::RPI::ModelAsset>& modelAsset) = 0;
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