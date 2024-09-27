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

#include <AzCore/Component/Component.h>
#include <AzFramework/Terrain/TerrainDataRequestBus.h>
#include <AzFramework/Visibility/BoundsBus.h>
#include <Entity/Terrain/TerrainData.h>
#include <RGL/RGLBus.h>
#include <Wrappers/RglEntity.h>
#include <Wrappers/RglMesh.h>
#include <Wrappers/RglTexture.h>

namespace RGL
{
    struct TerrainIntensityConfiguration;

    //! Queries the TerrainDataRequestBus for terrain heights and constructs a mesh using them.
    //! Terrain area is split into square sectors of predetermined width.
    //! The constructed mesh has a uniform vertex distribution along the xy - plane.
    class TerrainEntityManagerSystemComponent
        : public AZ::Component
        , private AzFramework::Terrain::TerrainDataNotificationBus::Handler
        , private RGLNotificationBus::Handler
    {
    public:
        AZ_COMPONENT(TerrainEntityManagerSystemComponent, "{6de4556f-5621-4ec3-a587-b28988f79d8a}");
        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);

        TerrainEntityManagerSystemComponent() = default;

        // AzFramework::Terrain::TerrainDataNotificationBus overrides
        void OnTerrainDataCreateEnd() override;
        void OnTerrainDataDestroyEnd() override;
        void OnTerrainDataChanged(const AZ::Aabb& dirtyRegion, TerrainDataChangedMask dataChangedMask) override;

    protected:
        void Activate() override;
        void Deactivate() override;

    private:
        static Wrappers::RglTexture CreateTextureFromConfig(const TerrainIntensityConfiguration& intensityConfig);

        // RGLNotificationBus overrides
        void OnSceneConfigurationSet(const SceneConfiguration& config) override;

        void EnsureRGLEntityDestroyed();

        void UpdateWorldBounds();
        void UpdateDirtyRegion(const AZ::Aabb& dirtyRegion);

        Wrappers::RglMesh m_rglMesh = Wrappers::RglMesh::CreateInvalid();
        Wrappers::RglEntity m_rglEntity = Wrappers::RglEntity::CreateInvalid();
        Wrappers::RglTexture m_rglTexture = Wrappers::RglTexture::CreateInvalid();

        TerrainData m_terrainData;

        int32_t m_packedRglEntityId;
    };
} // namespace RGL
