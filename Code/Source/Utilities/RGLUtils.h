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
    class RglUtils {
    public:
        //! Handles RGL - related errors. Each status returned by RGL API function passed to this function.
        static void ErrorCheck(const rgl_status_t& status);

        static rgl_mat3x4f RglMat3x4FromAzMatrix3x4(const AZ::Matrix3x4& azMatrix);
        // TODO - name change!!
        static AZ::Matrix3x4 AzMatrix3x2FromRglMat3x4(const rgl_mat3x4f& rglMatrix);
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