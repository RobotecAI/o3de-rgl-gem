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

#include <Atom/RPI.Reflect/Image/StreamingImageAsset.h>
#include <AzCore/Asset/AssetCommon.h>
#include <AzCore/Component/Component.h>

namespace RGL
{
    //! Structure used to describe global terrain intensity configuration.
    struct TerrainIntensityConfiguration
    {
        AZ_TYPE_INFO(TerrainIntensityConfiguration, "{6cf06491-3d18-4aad-88f6-d1990d6f791f}");
        static void Reflect(AZ::ReflectContext* context);

        AZ::Data::Asset<AZ::RPI::StreamingImageAsset> m_colorImageAsset{ AZ::Data::AssetLoadBehavior::QueueLoad };
        float m_defaultValue{ 0.5f };
        bool m_isTiled{ true };
    };

    //! Structure used to describe all global scene parameters.
    struct SceneConfiguration
    {
        AZ_TYPE_INFO(SceneConfiguration, "{7e55de90-e26c-4567-9e06-822c6ce62b9c}");
        static void Reflect(AZ::ReflectContext* context);

        TerrainIntensityConfiguration m_terrainIntensityConfig;
        // clang-format off
        bool m_isSkinnedMeshUpdateEnabled{ true }; //!< If set to true, all skinned meshes will be updated. Otherwise they will remain unchanged.
        // clang-format on
    };

    class SceneConfigurationComponent : public AZ::Component
    {
    public:
        AZ_COMPONENT(SceneConfigurationComponent, "{18c28e89-27f0-4230-936e-d7858b39d53f}", AZ::Component);

        SceneConfigurationComponent() = default;
        ~SceneConfigurationComponent() override = default;

        static void Reflect(AZ::ReflectContext* context);

        // clang-format off
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType &required) {}
        // clang-format on

        // AZ::Component overrides
        void Activate() override;
        void Deactivate() override;

    private:
        SceneConfiguration m_config;
    };
} // namespace RGL
