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

#include <rgl/api/core.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/std/containers/unordered_map.h>

namespace RGL
{
    using ResultLayout = AZStd::vector<rgl_field_t>;

    //! Class allowing for easier storage of rgl raycast results.
    class RaycastResults
    {
    public:
        explicit RaycastResults(const ResultLayout& layout, size_t count);
        RaycastResults(const RaycastResults& other);
        RaycastResults(RaycastResults&& other);
        ~RaycastResults() = default;

        [[nodiscard]] size_t GetSize() const;
        [[nodiscard]] size_t GetCount() const;

        void* GetData();
        void* GetFieldPtr(size_t index, rgl_field_t fieldType);
        void Resize(size_t count);

        RaycastResults& operator=(const RaycastResults& other) = default;

    private:
        size_t m_count{ 0LU }; //!< Count of datapoints.
        size_t m_elementSize{ 0LU }; //!< Size of a single datapoint.

        AZStd::unordered_map<rgl_field_t, size_t> m_layout;

        // Buffer for result storage. We use uint8_t since its size is equal to 1 byte.
        AZStd::vector<uint8_t> m_data;
    };
} // namespace RGL
