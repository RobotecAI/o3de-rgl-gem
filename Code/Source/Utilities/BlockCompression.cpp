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
#include <Utilities/BlockCompression.h>

namespace RGL::Utils
{
    AZStd::pair<size_t, size_t> BC3Block::GetBlockIndices(uint32_t width, uint32_t x, uint32_t y)
    {
        return AZ::RPI::BC1Block::GetBlockIndices(width, x, y);
    }

    AZ::Color BC3Block::GetBlockColor(size_t pixelIndex) const
    {
        AZ::Color color = m_bc1.GetBlockColor(pixelIndex);
        color.SetA8(GetAlpha(pixelIndex));
        return color;
    }

    static AZ::u8 LerpU8(AZ::u8 val0, AZ::u8 val1, AZ::u8 i, AZ::u8 n)
    {
        return (val0 * i + val1 * (n - i)) / n;
    }

    AZ::u8 BC3Block::GetAlpha(size_t pixelIndex) const
    {
        AZ_Assert(pixelIndex < 16, "Unsupported pixel index for BC3: %zu", pixelIndex);
        static constexpr AZ::u8 IndexBitShift = 16U;
        static constexpr AZ::u8 IndexBitCount = 3U;

        const AZ::u8 valueBitShift = pixelIndex * IndexBitCount + IndexBitShift;
        const AZ::u8 index = (m_alphaData >> valueBitShift) & 0b111;
        if (m_referenceAlpha[0] > m_referenceAlpha[1])
        {
            switch (index)
            {
            case 0:
            case 1:
                return m_referenceAlpha[index];
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
                return LerpU8(m_referenceAlpha[1], m_referenceAlpha[0], index - 1U, 7U);
            }
        }
        else
        {
            switch (index)
            {
            case 0:
            case 1:
                return m_referenceAlpha[index];
            case 2:
            case 3:
            case 4:
            case 5:
                return LerpU8(m_referenceAlpha[1], m_referenceAlpha[0], index - 1U, 5U);
            case 6:
                return 0U;
            case 7:
                return 255U;
            }
        }

        return 0U;
    }
} // namespace RGL::Utils
