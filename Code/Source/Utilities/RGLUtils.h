/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

namespace RGL
{
    //! Class containing useful RGL - related utilities.
    class RglUtils
    {
    public:
        //! Creates an RGL mesh ensuring that if it cannot be created the targetMesh is set to nullptr.
        //! This function should be preferred over the rgl_mesh_create function.
        static void SafeRglMeshCreate(
            rgl_mesh_t& targetMesh, const rgl_vec3f* vertices, size_t vertexCount, const rgl_vec3i* indices, size_t indexCount);

        //! Creates an RGL entity ensuring that if it cannot be created the targetEntity is set to nullptr.
        //! This function should be preferred over the rgl_entity_create function.
        static void SafeRglEntityCreate(rgl_entity_t& targetEntity, rgl_mesh_t mesh);

        //! Returns whether provided status signifies a success and if not prints the last RGL error message.
        //! Each status returned by RGL API function should be passed to this function.
        //! @return Was this operation successful?
        static bool ErrorCheck(const rgl_status_t& status);

        static rgl_mat3x4f RglMat3x4FromAzMatrix3x4(const AZ::Matrix3x4& azMatrix);
        static AZ::Matrix3x4 AzMatrix3x4FromRglMat3x4(const rgl_mat3x4f& rglMatrix);
        static AZ::Vector3 AzVector3FromRglVec3f(const rgl_vec3f& rglVector);

        static constexpr rgl_mat3x4f IdentityTransform{
            .value{
                { 1, 0, 0, 0 },
                { 0, 1, 0, 0 },
                { 0, 0, 1, 0 },
            },
        };
    };
}; // namespace RGL