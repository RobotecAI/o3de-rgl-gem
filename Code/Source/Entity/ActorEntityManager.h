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

#include <Entity/EntityManager.h>
#include <Integration/ActorComponentBus.h>
#include <rgl/api/core.h>

namespace EMotionFX
{
    class Mesh;
    class ActorInstance;
}

namespace RGL
{
    class ActorEntityManager
        : public EntityManager
        , public EMotionFX::Integration::ActorComponentNotificationBus::Handler
    {
    public:
        explicit ActorEntityManager(AZ::EntityId entityId,
                                    AZStd::set<AZStd::pair<AZStd::string, uint8_t> > &class_tags);

        ActorEntityManager(const ActorEntityManager& other) = delete;
        ActorEntityManager(ActorEntityManager&& other) = delete;
        ActorEntityManager& operator=(ActorEntityManager&& rhs) = delete;
        ActorEntityManager& operator=(const ActorEntityManager&) = delete;
        ~ActorEntityManager();

        void Update() override;

    protected:
        // ActorComponentNotificationBus overrides
        void OnActorInstanceCreated(EMotionFX::ActorInstance* actorInstance) override;

    private:
        struct MeshPair
        {
            EMotionFX::Mesh* m_eMotionMesh; // might need to change (depends on its lifetime)
            rgl_mesh_t m_rglMesh;
        };

        EMotionFX::ActorInstance* m_actorInstance = nullptr;
        // We do not use the MeshLibrary since the actor mesh is
        // skinned and the mesh sharing would not be useful.
        AZStd::vector<MeshPair> m_meshes;
        AZStd::vector<rgl_vec3f> m_positions;

        void UpdateMeshVertices();
        void UpdateVertexPositions(const EMotionFX::Mesh& mesh);
        AZStd::vector<rgl_vec3i> CollectIndexData(const EMotionFX::Mesh& mesh);
        Mesh* EMotionFXMeshToRglMesh(const EMotionFX::Mesh& mesh);
    };
} // namespace RGL
