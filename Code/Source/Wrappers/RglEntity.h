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

#include <rgl/api/core.h>

namespace RGL::Wrappers
{
    class RglMesh;
    class RglTexture;

    class RglEntity
    {
    public:
        static RglEntity CreateInvalid()
        {
            return {};
        }

        explicit RglEntity(const RglMesh& mesh);
        RglEntity(const RglEntity& other) = delete;
        RglEntity(RglEntity&& other);
        ~RglEntity();

        [[nodiscard]] bool IsValid() const
        {
            return m_nativePtr;
        }

        void SetPose(const rgl_mat3x4f& pose);
        void SetId(int32_t id);
        void SetIntensityTexture(const RglTexture& texture);

        RglEntity& operator=(const RglEntity& other) = delete;
        RglEntity& operator=(RglEntity&& other);

    private:
        //! Creates an invalid entity.
        //! To avoid creating an invalid entity by accident, it is private.
        //! See CreateInvalid.
        RglEntity() = default;

        rgl_entity_t m_nativePtr{ nullptr };
    };
} // namespace RGL::Wrappers
