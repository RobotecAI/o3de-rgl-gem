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

#include "LidarRaycaster.h"
#include "ROS2/Lidar/LidarSystemBus.h"

namespace RGL
{
    class LidarSystem : protected ROS2::LidarSystemRequestBus::Handler
    {
    public:
        LidarSystem() = default;
        LidarSystem(LidarSystem&& lidarSystem) = delete;
        LidarSystem(const LidarSystem& lidarSystem) = delete;
        ~LidarSystem() = default;

        void Activate();
        void Deactivate();

        //! Deletes all lidar raycasters created by this system.
        void Clear();

    protected:
        ////////////////////////////////////////////////////////////////////////
        // LidarSystemRequestBus::Handler interface implementation
        AZ::Uuid CreateLidar(const AZ::EntityId& lidarEntityId) override;
        ////////////////////////////////////////////////////////////////////////
    private:
        AZStd::vector<LidarRaycaster> m_lidars;
    };
} // namespace RGL
