/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#include <AzCore/Math/Matrix4x4.h>

#include <rgl/api/core.h>

#include "RGLUtils.h"

namespace RGL
{
    void RglUtils::SafeRglMeshCreate(
        rgl_mesh_t& targetMesh, const rgl_vec3f* vertices, size_t vertexCount, const rgl_vec3i* indices, size_t indexCount)
    {
        bool success = RglUtils::ErrorCheck(
            rgl_mesh_create(&targetMesh, vertices, aznumeric_cast<int32_t>(vertexCount), indices, aznumeric_cast<int32_t>(indexCount)));
        if (!success && targetMesh != nullptr)
        {
            RglUtils::ErrorCheck(rgl_mesh_destroy(targetMesh));
            targetMesh = nullptr;
        }
    }

    void RglUtils::SafeRglEntityCreate(rgl_entity_t& targetEntity, rgl_mesh_t mesh)
    {
        bool success = RglUtils::ErrorCheck(rgl_entity_create(&targetEntity, nullptr, mesh));
        if (!success && targetEntity != nullptr)
        {
            RglUtils::ErrorCheck(rgl_entity_destroy(targetEntity));
            targetEntity = nullptr;
        }
    }

    bool RglUtils::ErrorCheck(const rgl_status_t& status)
    {
        static const AZStd::unordered_set<rgl_status_t> UnrecoverableStates = { RGL_INVALID_STATE,
                                                                                RGL_INTERNAL_EXCEPTION,
                                                                                RGL_INVALID_PIPELINE };

        if (status == RGL_SUCCESS)
        {
            return true;
        }

        const char* errorString;
        rgl_get_last_error_string(&errorString);
        if (UnrecoverableStates.contains(status))
        {
            AZ_Assert(false, std::string("RGL encountered an unrecoverable error with message: ").append(errorString).c_str());
        }

        AZ_Error(__func__, false, std::string("RGL encountered an error with message: ").append(errorString).c_str());
        return false;
    }

    rgl_mat3x4f RglUtils::RglMat3x4FromAzMatrix3x4(const AZ::Matrix3x4& azMatrix)
    {
        return {
            {
                { azMatrix.GetRow(0).GetX(), azMatrix.GetRow(0).GetY(), azMatrix.GetRow(0).GetZ(), azMatrix.GetRow(0).GetW() },
                { azMatrix.GetRow(1).GetX(), azMatrix.GetRow(1).GetY(), azMatrix.GetRow(1).GetZ(), azMatrix.GetRow(1).GetW() },
                { azMatrix.GetRow(2).GetX(), azMatrix.GetRow(2).GetY(), azMatrix.GetRow(2).GetZ(), azMatrix.GetRow(2).GetW() },
            },
        };
    }

    AZ::Matrix3x4 RglUtils::AzMatrix3x4FromRglMat3x4(const rgl_mat3x4f& rglMatrix)
    {
        float m_rowMajorValues[]{
            rglMatrix.value[0][0], rglMatrix.value[0][1], rglMatrix.value[0][2], rglMatrix.value[0][3],
            rglMatrix.value[1][0], rglMatrix.value[1][1], rglMatrix.value[1][2], rglMatrix.value[1][3],
            rglMatrix.value[2][0], rglMatrix.value[2][1], rglMatrix.value[2][2], rglMatrix.value[2][3],
        };
        return AZ::Matrix3x4::CreateFromRowMajorFloat12(m_rowMajorValues);
    }

    AZ::Vector3 RglUtils::AzVector3FromRglVec3f(const rgl_vec3f& rglVector)
    {
        return AZ::Vector3(rglVector.value[0], rglVector.value[1], rglVector.value[2]);
    }
}; // namespace RGL
