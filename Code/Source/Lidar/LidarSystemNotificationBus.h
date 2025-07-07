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

#include <AzCore/Component/EntityId.h>
#include <AzCore/EBus/EBus.h>
#include <ROS2/Lidar/LidarSystemBus.h>
#include <ROS2/Lidar/LidarRaycasterBus.h>

namespace RGL
{
    class LidarSystemNotifications : public AZ::EBusTraits
    {
    public:
        virtual ~LidarSystemNotifications() = default;

        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////

        virtual void OnLidarCreated()
        {
        }

        virtual void OnLidarDestroyed()
        {
        }
        //////////////////////////////////////////////////////////////////////////
    };
    using LidarSystemNotificationBus = AZ::EBus<LidarSystemNotifications>;
} // namespace RGL
