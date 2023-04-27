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
#include <Lidar/RaycastResults.h>

namespace RGL
{
    RaycastResults::RaycastResults(const ResultLayout& layout, size_t count)
        : m_count{ count }
    {
        AZ_Error(__func__, !layout.empty(), "Incorrect layout - ResultLayout must not be empty.");

        m_elementSize = 0LU;
        bool duplicateFound = false;
        for (const auto& field : layout)
        {
            switch (field)
            {
            case RGL_FIELD_XYZ_F32:
                duplicateFound = !m_layout.emplace(RGL_FIELD_XYZ_F32, m_elementSize).second;
                m_elementSize += sizeof(rgl_vec3f);
                break;
            case RGL_FIELD_IS_HIT_I32:
                duplicateFound = !m_layout.emplace(RGL_FIELD_IS_HIT_I32, m_elementSize).second;
                m_elementSize += sizeof(int32_t);
                break;
            case RGL_FIELD_DISTANCE_F32:
                duplicateFound = !m_layout.emplace(RGL_FIELD_DISTANCE_F32, m_elementSize).second;
                m_elementSize += sizeof(float);
                break;
            }

            AZ_Error(__func__, !duplicateFound, "Incorrect layout - ResultLayout contains duplicate fields.")
        }

        size_t size = m_elementSize * count;
        m_data = AZStd::vector<uint8_t>(size);
    }

    RaycastResults::RaycastResults(const RaycastResults& other)
        : m_count{ other.m_count }
        , m_elementSize{ other.m_elementSize }
        , m_layout{ other.m_layout }
        , m_data{ other.m_data }
    {
    }

    RaycastResults::RaycastResults(RaycastResults&& other)
        : m_count{ other.m_count }
        , m_elementSize{ other.m_elementSize }
        , m_layout{ AZStd::move(other.m_layout) }
        , m_data{ AZStd::move(other.m_data) }
    {
        other.m_count = 0LU;
        other.m_elementSize = 0LU;
        other.m_layout = {};
        other.m_data = {};
    }

    size_t RaycastResults::GetSize() const
    {
        return m_data.size();
    }

    size_t RGL::RaycastResults::GetCount() const
    {
        return m_count;
    }

    void* RaycastResults::GetData()
    {
        return static_cast<void*>(m_data.data());
    }

    void RaycastResults::Resize(size_t count)
    {
        m_count = count;
        m_data.resize(count * m_elementSize);
    }

    void* RaycastResults::GetFieldPtr(size_t index, rgl_field_t fieldType)
    {
        const auto fieldOffset = m_layout.find(fieldType);
        AZ_Error(__func__, fieldOffset != m_layout.end(), "Field of specified type is not contained within result layout.");

        return static_cast<void*>(m_data.data() + (m_elementSize * index) + fieldOffset->second);
    }
} // namespace RGL
