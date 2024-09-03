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

#include <Entity/ActorEntityManager.h>

#include <AzCore/std/string/string.h>
#include <EMotionFX/Source/ActorInstance.h>
#include <EMotionFX/Source/Mesh.h>
#include <EMotionFX/Source/Node.h>
#include <EMotionFX/Source/SubMesh.h>
#include <EMotionFX/Source/TransformData.h>
#include <RGL/RGLBus.h>
#include <Utilities/RGLUtils.h>
#include <Wrappers/RglEntity.h>
#include <Wrappers/RglMesh.h>
#include <rgl/api/core.h>

namespace RGL
{
    ActorEntityManager::ActorEntityManager(AZ::EntityId entityId)
        : EntityManager(entityId)
    {
        EMotionFX::Integration::ActorComponentNotificationBus::Handler::BusConnect(entityId);
    }

    void ActorEntityManager::Update()
    {
        if (RGLInterface::Get()->GetSceneConfiguration().m_isSkinnedMeshUpdateEnabled)
        {
            UpdateMeshVertices();
        }
        EntityManager::Update();
    }

    void ActorEntityManager::OnActorInstanceCreated(EMotionFX::ActorInstance* actorInstance)
    {
        m_actorInstance = actorInstance;
        EMotionFX::Actor* actor = actorInstance->GetActor();
        [[maybe_unused]] const AZ::Entity* ActorEntity = actorInstance->GetEntity();

        static constexpr size_t LodLevel = 0UL; // Highest LOD.
        const size_t NodeCount = actor->GetNumNodes();
        for (size_t jointIndex = 0LU; jointIndex < NodeCount; ++jointIndex)
        {
            EMotionFX::Mesh* mesh = actor->GetMesh(LodLevel, jointIndex);
            if (!mesh)
            {
                continue;
            }

            Wrappers::RglMesh rglMesh = AZStd::move(EMotionFXMeshToRglMesh(*mesh));
            if (rglMesh.IsValid())
            {
                m_emotionFxMeshes.emplace_back(mesh);
                m_rglMeshes.emplace_back(AZStd::move(rglMesh));
            }
            else
            {
                AZ_Error(
                    __func__,
                    false,
                    "[Entity: %s:%s] Unable to create rgl mesh for actor instance.",
                    ActorEntity->GetName().c_str(),
                    ActorEntity->GetId().ToString().c_str());
            }
        }

        m_entities.reserve(m_emotionFxMeshes.size());
        for (const Wrappers::RglMesh& rglMesh : m_rglMeshes)
        {
            Wrappers::RglEntity entity(rglMesh);
            if (entity.IsValid())
            {
                if (m_packedRglEntityId.has_value())
                {
                    entity.SetId(m_packedRglEntityId.value());
                }
                m_entities.emplace_back(AZStd::move(entity));
            }
            else
            {
                AZ_Error(
                    __func__,
                    false,
                    "[Entity: %s:%s] Unable to create rgl entity for actor instance.",
                    ActorEntity->GetName().c_str(),
                    ActorEntity->GetId().ToString().c_str());
            }
        }

        m_isPoseUpdateNeeded = true;
    }

    void ActorEntityManager::UpdateMeshVertices()
    {
        m_actorInstance->UpdateMeshDeformers(0.0f);

        for (size_t i = 0; i < m_emotionFxMeshes.size(); ++i)
        {
            UpdateVertexPositions(*m_emotionFxMeshes[i]);
            m_rglMeshes[i].UpdateVertices(m_positions.data(), m_positions.size());
        }
    }

    void ActorEntityManager::UpdateVertexPositions(const EMotionFX::Mesh& mesh)
    {
        const size_t VertexCount = mesh.GetNumVertices();
        auto* vertices = static_cast<const AZ::Vector3*>(mesh.FindVertexData(EMotionFX::Mesh::ATTRIB_POSITIONS));

        m_positions.clear();
        m_positions.reserve(VertexCount);
        for (size_t vertex = 0; vertex < VertexCount; ++vertex)
        {
            m_positions.push_back(Utils::RglVector3FromAzVec3f(vertices[vertex]));
        }
    }

    AZStd::vector<rgl_vec3i> ActorEntityManager::CollectIndexData(const EMotionFX::Mesh& mesh)
    {
        const size_t IndexCount = mesh.GetNumIndices();
        const size_t TriangleCount = IndexCount / 3LU;

        uint32* indices = mesh.GetIndices();
        AZStd::vector<rgl_vec3i> rglIndices;
        rglIndices.reserve(TriangleCount);

        const size_t SubMeshCount = mesh.GetNumSubMeshes();
        for (size_t subMeshNr = 0; subMeshNr < SubMeshCount; ++subMeshNr)
        {
            EMotionFX::SubMesh* subMesh = mesh.GetSubMesh(subMeshNr);
            const size_t SubMeshIndexCount = subMesh->GetNumIndices();
            const size_t SubMeshTriangleCount = SubMeshIndexCount / 3LU;

            const size_t IndexBase = subMesh->GetStartIndex();
            const size_t VertexBase = subMesh->GetStartVertex();
            for (size_t triangle = 0LU; triangle < SubMeshTriangleCount; ++triangle)
            {
                const size_t TriangleFirstIndex = triangle * 3LU + IndexBase;

                // Note: We cast the uint32 to int32_t but this conversion is unlikely to result in negative values.
                rglIndices.push_back({
                    aznumeric_cast<int32_t>(VertexBase + indices[TriangleFirstIndex]),
                    aznumeric_cast<int32_t>(VertexBase + indices[TriangleFirstIndex + 1LU]),
                    aznumeric_cast<int32_t>(VertexBase + indices[TriangleFirstIndex + 2LU]),
                });
            }
        }

        return rglIndices;
    }

    Wrappers::RglMesh ActorEntityManager::EMotionFXMeshToRglMesh(const EMotionFX::Mesh& mesh)
    {
        UpdateVertexPositions(mesh);
        const AZStd::vector<rgl_vec3i> RglIndices = CollectIndexData(mesh);
        return { m_positions.data(), m_positions.size(), RglIndices.data(), RglIndices.size() };
    }
} // namespace RGL
