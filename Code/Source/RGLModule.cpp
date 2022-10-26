/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#include "RGLSystemComponent.h"
#include <RGLModuleInterface.h>

namespace RGL
{
    class RGLModule : public RGLModuleInterface
    {
    public:
        AZ_RTTI(RGLModule, "{06ADDE3E-333C-49E7-A63F-BE3E8F3CFB4C}", RGLModuleInterface);
        AZ_CLASS_ALLOCATOR(RGLModule, AZ::SystemAllocator, 0);
    };
} // namespace RGL

AZ_DECLARE_MODULE_CLASS(Gem_RGL, RGL::RGLModule)
