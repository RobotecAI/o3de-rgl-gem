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
