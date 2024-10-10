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

#include <AtomLyIntegration/CommonFeatures/Material/MaterialComponentBus.h>
#include <AzCore/std/containers/vector.h>
#include <Entity/MaterialEntityManager.h>
#include <Integration/ActorComponentBus.h>
#include <Wrappers/RglMesh.h>
#include <rgl/api/core.h>

namespace EMotionFX
{
    class Mesh;
    class ActorInstance;
} // namespace EMotionFX

namespace RGL
{
    class ActorEntityManager
        : public MaterialEntityManager
        , public EMotionFX::Integration::ActorComponentNotificationBus::Handler
    {
    public:
        explicit ActorEntityManager(AZ::EntityId entityId);
        ActorEntityManager(const ActorEntityManager& other) = delete;
        ActorEntityManager(ActorEntityManager&& other) = delete;
        ActorEntityManager& operator=(ActorEntityManager&& rhs) = delete;
        ActorEntityManager& operator=(const ActorEntityManager&) = delete;
        ~ActorEntityManager();

        void Update() override;

    protected:
        // ActorComponentNotificationBus overrides
        void OnActorInstanceCreated(EMotionFX::ActorInstance* actorInstance) override;
        void OnActorInstanceDestroyed(EMotionFX::ActorInstance* actorInstance) override;

        // AZ::EntityBus::Handler implementation overrides
        void OnEntityDeactivated(const AZ::EntityId& entityId) override;

    private:
        static AZStd::vector<rgl_vec3i> CollectIndexData(const EMotionFX::Mesh& mesh);
        static AZStd::vector<rgl_vec3f> GetMeshVertexPositions(const EMotionFX::Mesh& mesh);
        AZStd::optional<AZStd::vector<rgl_vec2f>> CollectUvData(const EMotionFX::Mesh& mesh) const;

        void UpdateMaterialSlots(const EMotionFX::Actor& actor);
        void UpdateMeshVertices();
        //! Loads mesh's vertex position data into the m_tempVertexPositions buffer.
        bool ProcessEfxMesh(const EMotionFX::Mesh& mesh);

        EMotionFX::ActorInstance* m_actorInstance = nullptr;
        // We do not use the ModelLibrary since the actor mesh is
        // skinned and the mesh sharing would not be useful.
        EMotionFX::Mesh* m_emotionFxMesh;
        AZStd::vector<Wrappers::RglMesh> m_rglSubMeshes;
    };
} // namespace RGL
