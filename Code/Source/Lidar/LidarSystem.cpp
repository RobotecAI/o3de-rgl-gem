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
#include <Lidar/LidarSystem.h>
#include <ROS2/Lidar/LidarRegistrarBus.h>

namespace RGL
{
    LidarSystem::LidarSystem(LidarSystem&& lidarSystem)
        : m_lidars{ AZStd::move(lidarSystem.m_lidars) }
    {
        lidarSystem.BusDisconnect();
    }

    void LidarSystem::Activate()
    {
        const char* name = "RobotecGPULidar";
        const char* description = "Mesh-based lidar implementation that uses the RobotecGPULidar API for GPU-enabled raycasting.";

        using Features = ROS2::LidarSystemFeatures;
        static constexpr auto SupportedFeatures = aznumeric_cast<Features>(
            Features::EntityExclusion | Features::MaxRangePoints | Features::Noise | Features::Intensity | Features::Segmentation);

        ROS2::LidarSystemRequestBus::Handler::BusConnect(AZ_CRC(name));

        auto* lidarSystemManagerInterface = ROS2::LidarRegistrarInterface::Get();
        AZ_Assert(lidarSystemManagerInterface != nullptr, "The ROS2 LidarSystem Manager interface was inaccessible.");
        lidarSystemManagerInterface->RegisterLidarSystem(name, description, SupportedFeatures);
    }

    void LidarSystem::Deactivate()
    {
        ROS2::LidarSystemRequestBus::Handler::BusDisconnect();
    }

    void LidarSystem::Clear()
    {
        m_lidars.clear();
    }

    ROS2::LidarId LidarSystem::CreateLidar(AZ::EntityId lidarEntityId)
    {
        const AZ::Uuid lidarUuid = AZ::Uuid::CreateRandom();
        m_lidars.emplace(lidarUuid, LidarRaycaster(lidarUuid));
        return ROS2::LidarId(lidarUuid);
    }

    void LidarSystem::DestroyLidar(ROS2::LidarId lidarId)
    {
        m_lidars.erase(lidarId);
    }
} // namespace RGL