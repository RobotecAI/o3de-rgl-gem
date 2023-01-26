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

namespace RGL
{
    class RGLRequests
    {
    public:
        AZ_RTTI(RGLRequests, "{f301342c-3b17-11ed-a261-0242ac120002}");

        //! Excludes an entity from raycasting.
        //! @param excludedEntityId EntityId of the excluded entity.
        virtual void ExcludeEntity(const AZ::EntityId& excludedEntityId) = 0;

    protected:
        ~RGLRequests() = default;
    };

    class RGLBusTraits : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using RGLRequestBus = AZ::EBus<RGLRequests, RGLBusTraits>;
    using RGLInterface = AZ::Interface<RGLRequests>;
} // namespace RGL
