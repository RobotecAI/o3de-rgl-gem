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
#include <AzCore/std/string/string.h>

namespace RGL
{
    class RGLRequests
    {
    public:
        AZ_RTTI(RGLRequests, "{f301342c-3b17-11ed-a261-0242ac120002}");
        virtual ~RGLRequests() = default;
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
