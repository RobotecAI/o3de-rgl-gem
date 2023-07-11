/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

#include <rgl/api/core.h>
#include <AzCore/Math/Matrix3x4.h>

namespace RGL::Utils
{
    //! Creates an RGL mesh ensuring that if it cannot be created the targetMesh is set to nullptr.
    //! This function should be preferred over the rgl_mesh_create function.
    void SafeRglMeshCreate(
        rgl_mesh_t& targetMesh, const rgl_vec3f* vertices, size_t vertexCount, const rgl_vec3i* indices, size_t indexCount);

    //! Creates an RGL entity ensuring that if it cannot be created the targetEntity is set to nullptr.
    //! This function should be preferred over the rgl_entity_create function.
    void SafeRglEntityCreate(rgl_entity_t& targetEntity, rgl_mesh_t mesh);

    //! If the provided status signifies an error, prints the last RGL error message.
    //! @param status Status returned by an API call.
    //! @param file String representing the file path of the file in which the API call was made
    //! @param line Line at which the API call was made
    //! @param line Line at which the API call was made
    //! @param successDest Optional parameter for the destination of boolean value representing success.
    //! The value is not written if the pointer is set to nullptr.
    void ErrorCheck(const rgl_status_t& status, const char* file, int line, bool* successDest = nullptr);

    //! Macro used for calling the ErrorCheck function.
    //! Each status returned by RGL API should be passed to it.
#define RGL_CHECK(x) RGL::Utils::ErrorCheck(x, __FILE__, __LINE__)

    rgl_mat3x4f RglMat3x4FromAzMatrix3x4(const AZ::Matrix3x4& azMatrix);
    AZ::Matrix3x4 AzMatrix3x4FromRglMat3x4(const rgl_mat3x4f& rglMatrix);
    AZ::Vector3 AzVector3FromRglVec3f(const rgl_vec3f& rglVector);
    rgl_vec3f RglVector3FromAzVec3f(const AZ::Vector3& azVector);

    constexpr rgl_mat3x4f IdentityTransform{
        .value{
            { 1, 0, 0, 0 },
            { 0, 1, 0, 0 },
            { 0, 0, 1, 0 },
        },
    };
} // namespace RGL::Utils