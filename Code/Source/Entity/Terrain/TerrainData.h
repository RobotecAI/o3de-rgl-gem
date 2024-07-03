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

#include <AzCore/Math/Aabb.h>

namespace RGL
{
    struct TerrainData
    {
        AZ::Aabb m_currentWorldBounds = { AZ::Aabb::CreateFromPoint(AZ::Vector3::CreateZero()) };
        AZStd::vector<rgl_vec3f> m_vertices;
        AZStd::vector<rgl_vec3i> m_indices;

        //! Returns whether or not newWorldBounds resulted in terrain update.
        bool UpdateBounds(AZ::Aabb newWorldBounds);
        void UpdateDirtyRegion(const AZ::Aabb& dirtyRegion);

        void Clear();

    private:
        static constexpr size_t TrianglesPerSector = 2LU;
    };

} // namespace RGL
