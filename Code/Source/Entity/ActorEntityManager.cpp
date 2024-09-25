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
#include <EMotionFX/Source/Actor.h>
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
        : MaterialEntityManager(entityId)
    {
        AZ::EntityBus::Handler::BusConnect(m_entityId);
        EMotionFX::Integration::ActorComponentNotificationBus::Handler::BusConnect(entityId);
    }

    ActorEntityManager::~ActorEntityManager()
    {
        AZ::EntityBus::Handler::BusDisconnect();
    }

    void ActorEntityManager::Update()
    {
        if (!m_entities.empty() && RGLInterface::Get()->GetSceneConfiguration().m_isSkinnedMeshUpdateEnabled)
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

        static constexpr size_t LodLevel = 0U; // Highest LOD.
        static constexpr size_t NodeIdx = 0U; // Default mesh node.
        EMotionFX::Mesh* mesh = actor->GetMesh(LodLevel, NodeIdx);
        if (!mesh)
        {
            AZ_Assert(false, "No mesh found at joint 0. Unable to process the actor instance.");
            return;
        }

        if (ProcessEfxMesh(*mesh))
        {
            m_emotionFxMesh = mesh;
            UpdateMaterialSlots(*actor);
            m_isPoseUpdateNeeded = true;
        }
    }

    void ActorEntityManager::OnActorInstanceDestroyed(EMotionFX::ActorInstance* actorInstance)
    {
        AZ::Render::MaterialComponentNotificationBus::Handler::BusDisconnect();
        ResetMaterialsMapping();
        m_entities.clear();
        m_rglSubMeshes.clear();
        m_emotionFxMesh = nullptr;
    }

    void ActorEntityManager::OnEntityDeactivated(const AZ::EntityId& entityId)
    {
        AZ::Render::MaterialComponentNotificationBus::Handler::BusDisconnect();
        EMotionFX::Integration::ActorComponentNotificationBus::Handler::BusDisconnect();
        MaterialEntityManager::OnEntityDeactivated(entityId);
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
            for (size_t triangle = 0LU; triangle < SubMeshTriangleCount; ++triangle)
            {
                const size_t TriangleFirstIndex = triangle * 3LU + IndexBase;

                // Note: We cast the uint32 to int32_t but this conversion is unlikely to result in negative values.
                rglIndices.push_back({
                    aznumeric_cast<int32_t>(indices[TriangleFirstIndex]),
                    aznumeric_cast<int32_t>(indices[TriangleFirstIndex + 1LU]),
                    aznumeric_cast<int32_t>(indices[TriangleFirstIndex + 2LU]),
                });
            }
        }

        return rglIndices;
    }

    AZStd::optional<AZStd::vector<rgl_vec2f>> ActorEntityManager::CollectUvData(const EMotionFX::Mesh& mesh) const
    {
        const size_t VertexCount = mesh.GetNumVertices();
        AZStd::vector<rgl_vec2f> rglUvs(VertexCount);

        const auto* uvs = static_cast<const AZ::Vector2*>(mesh.FindVertexData(EMotionFX::Mesh::ATTRIB_UVCOORDS));
        if (uvs == nullptr)
        {
            AZ_Error(
                __func__, false, "Unable to collect UV data from an actor component of entity with ID: %s.", m_entityId.ToString().c_str());

            return AZStd::nullopt;
        }

        auto uvSpan = AZStd::span(uvs, VertexCount);
        AZStd::transform(uvSpan.begin(), uvSpan.end(), rglUvs.begin(), Utils::RglVec2fFromAzVector2);
        return rglUvs;
    }

    void ActorEntityManager::UpdateMaterialSlots(const EMotionFX::Actor& actor)
    {
        const AZ::Data::Asset<AZ::RPI::ModelAsset>& modelAsset = actor.GetMeshAsset();
        const auto lodAssets = modelAsset->GetLodAssets();
        // Get Highest LOD
        const auto modelLodAsset = lodAssets.begin()->Get();
        const auto meshes = modelLodAsset->GetMeshes();

        // Each mesh of the model is associated with one EFX sub mesh.
        for (size_t subMeshIdx = 0; subMeshIdx < meshes.size(); ++subMeshIdx)
        {
            const AZ::RPI::ModelMaterialSlot& slot = modelAsset->FindMaterialSlot(meshes[subMeshIdx].GetMaterialSlotId());
            AssignMaterialSlotIdForMesh(slot.m_stableId, subMeshIdx);
        }

        // We can use material info only when the model is ready.
        AZ::Render::MaterialComponentNotificationBus::Handler::BusConnect(m_entityId);
    }

    void ActorEntityManager::UpdateMeshVertices()
    {
        if (!m_emotionFxMesh)
        {
            return;
        }

        m_actorInstance->UpdateMeshDeformers(0.0f);

        LoadMeshVertexPositions(*m_emotionFxMesh);
        const size_t subMeshCount = m_emotionFxMesh->GetNumSubMeshes();
        for (size_t subMeshNr = 0; subMeshNr < subMeshCount; ++subMeshNr)
        {
            const EMotionFX::SubMesh* subMesh = m_emotionFxMesh->GetSubMesh(subMeshNr);
            const size_t VertexBase = subMesh->GetStartVertex();
            const size_t SubMeshVertexCount = subMesh->GetNumVertices();

            m_rglSubMeshes[subMeshNr].UpdateVertices(m_tempVertexPositions.data() + VertexBase, SubMeshVertexCount);
        }
    }

    void ActorEntityManager::LoadMeshVertexPositions(const EMotionFX::Mesh& mesh)
    {
        const size_t VertexCount = mesh.GetNumVertices();
        const auto* vertices = static_cast<const AZ::Vector3*>(mesh.FindVertexData(EMotionFX::Mesh::ATTRIB_POSITIONS));
        const auto vertexSpan = AZStd::span(vertices, VertexCount);

        m_tempVertexPositions.resize(VertexCount);
        AZStd::transform(vertexSpan.begin(), vertexSpan.end(), m_tempVertexPositions.begin(), Utils::RglVector3FromAzVec3f);
    }

    bool ActorEntityManager::ProcessEfxMesh(const EMotionFX::Mesh& mesh)
    {
        LoadMeshVertexPositions(mesh);

        const size_t subMeshCount = mesh.GetNumSubMeshes();
        const auto indices = CollectIndexData(mesh);
        const auto uvData = CollectUvData(mesh);
        for (size_t subMeshNr = 0; subMeshNr < subMeshCount; ++subMeshNr)
        {
            const EMotionFX::SubMesh* subMesh = mesh.GetSubMesh(subMeshNr);
            const size_t VertexBase = subMesh->GetStartVertex();
            const size_t VertexCount = subMesh->GetNumVertices();

            const size_t IndexBase = subMesh->GetStartIndex() / 3U; // RGL uses index triples.
            const size_t IndexCount = subMesh->GetNumIndices() / 3U;

            auto rglMesh =
                Wrappers::RglMesh(m_tempVertexPositions.data() + VertexBase, VertexCount, indices.data() + IndexBase, IndexCount);

            if (rglMesh.IsValid())
            {
                if (uvData.has_value())
                {
                    rglMesh.SetTextureCoordinates(uvData->data() + VertexBase, VertexCount);
                }

                m_rglSubMeshes.push_back(AZStd::move(rglMesh));
            }
            else
            {
                m_rglSubMeshes.clear();
                AZ_Error(
                    "RGL",
                    false,
                    "[Entity: %s] Rgl mesh creation failed for one of the sub meshes of an entity with an actor component. Some geometry "
                    "will not be present.",
                    m_entityId.ToString().c_str());
                return false;
            }
        }

        m_rglSubMeshes.reserve(m_rglSubMeshes.size());
        for (size_t i = 0; i < m_rglSubMeshes.size(); ++i)
        {
            if (Wrappers::RglEntity subMeshEntity(m_rglSubMeshes[i]); subMeshEntity.IsValid())
            {
                m_entities.emplace_back(AZStd::move(subMeshEntity));
            }
            else
            {
                m_rglSubMeshes.clear();
                m_entities.clear();
                AZ_Error(
                    "RGL",
                    false,
                    "[Entity: %s] Rgl entity creation failed for one of the sub meshes of an entity with an actor component. Some geometry "
                    "will not be present.",
                    m_entityId.ToString().c_str());
                return false;
            }
        }

        return true;
    }
} // namespace RGL
