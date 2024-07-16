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

#include <Atom/RPI.Reflect/Material/MaterialAsset.h>
#include <Atom/RPI.Reflect/Model/ModelAsset.h>
#include <AzCore/Asset/AssetCommon.h>
#include <AzCore/Component/EntityId.h>
#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

namespace RGL
{
    namespace Wrappers
    {
        class Texture;
        class Mesh;
    } // namespace Wrappers

    using MeshMaterialSlotPairList = AZStd::vector<AZStd::pair<Wrappers::Mesh, AZ::RPI::ModelMaterialSlot>>;

    class ModelLibraryRequests
    {
    public:
        AZ_RTTI(ModelLibraryRequests, "{b84ccaae-5d0f-410a-821e-5ff8d449b851}");

        //! Returns a vector of RGL meshes created from the modelAsset.
        //! If the provided modelAsset was not encountered before, created RGL meshes are stored by the library.
        //! On the other hand if the RGL meshes associated with the provided modelAsset were stored it will simply retrieve them.
        //! @param modelAsset Model asset provided for storage.
        //! @return List of RGL meshes created using the provided model asset.
        virtual const MeshMaterialSlotPairList& StoreModelAsset(const AZ::Data::Asset<AZ::RPI::ModelAsset>& modelAsset) = 0;

        //! Returns the texture created using provided materialAsset.
        //! The returned texture reference may point to an invalid texture.
        virtual const Wrappers::Texture& StoreMaterialAsset(const AZ::Data::Asset<AZ::RPI::MaterialAsset>& materialAsset) = 0;

    protected:
        ~ModelLibraryRequests() = default;
    };

    class ModelLibraryBusTraits : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using ModelLibraryRequestBus = AZ::EBus<ModelLibraryRequests, ModelLibraryBusTraits>;
    using ModelLibraryInterface = AZ::Interface<ModelLibraryRequests>;

    // class ModelLibraryNotifications
    //    : public AZ::EBusTraits
    // {
    // public:
    //     //////////////////////////////////////////////////////////////////////////
    //     // EBusTraits overrides
    //     static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
    //     static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
    //     //////////////////////////////////////////////////////////////////////////
    //
    //     //////////////////////////////////////////////////////////////////////////
    //     // Notifications interface
    //     virtual void OnMaterialUpdated(const AZ::Data::AssetId& materialAssetId);
    //     //////////////////////////////////////////////////////////////////////////
    //
    // };
    // using AWSCognitoAuthorizationNotificationBus = AZ::EBus<AWSCognitoAuthorizationNotifications>;
} // namespace RGL