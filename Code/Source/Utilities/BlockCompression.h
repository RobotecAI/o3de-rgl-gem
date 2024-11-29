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
#include <Atom/RPI.Public/BlockCompression.h>
#include <AzCore/Math/Color.h>
#include <AzCore/base.h>

namespace RGL::Utils
{
    //! Structure used for decompressing images compressed using BC3.
    //! To learn more about BC3 compression see
    //! https://learn.microsoft.com/en-us/windows/win32/direct3d10/d3d10-graphics-programming-guide-resources-block-compression#bc3.
    struct BC3Block
    {
        static AZStd::pair<size_t, size_t> GetBlockIndices(uint32_t width, uint32_t x, uint32_t y);

        [[nodiscard]] AZ::Color GetBlockColor(size_t pixelIndex) const;
        [[nodiscard]] AZ::u8 GetAlpha(size_t pixelIndex) const;

        union {
            AZ::u64 m_alphaData;
            struct
            {
                AZ::u8 m_referenceAlpha[2];
                AZ::u8 m_alphaIndices[6];
            };
        };
        AZ::RPI::BC1Block m_bc1;
    };
} // namespace RGL::Utils
