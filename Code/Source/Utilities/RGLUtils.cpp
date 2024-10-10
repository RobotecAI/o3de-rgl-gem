/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#include <AzCore/Casting/numeric_cast.h>
#include <AzCore/std/containers/unordered_set.h>
#include <AzCore/std/string/conversions.h>
#include <Utilities/RGLUtils.h>
#include <iostream>
#include <rgl/api/core.h>

namespace RGL::Utils
{
    void ErrorCheck(const rgl_status_t& status, const char* file, int line, bool* successDest)
    {
        static const AZStd::unordered_set<rgl_status_t> UnrecoverableStatuses = {
            RGL_INVALID_STATE,
            RGL_LOGGING_ERROR,
            RGL_INITIALIZATION_ERROR,
            RGL_INTERNAL_EXCEPTION,
        };

        if (status == RGL_SUCCESS)
        {
            if (successDest)
            {
                *successDest = true;
            }
            return;
        }

        AZStd::string errorLocationString{ AZStd::string{ "in file " }.append(file).append(" at line ").append(AZStd::to_string(line)) };
        const char* errorString;
        rgl_get_last_error_string(&errorString);
        if (UnrecoverableStatuses.contains(status))
        {
            AZ_Assert(
                false,
                std::string("RGL encountered an unrecoverable error ")
                    .append(errorLocationString.c_str())
                    .append(" with message: ")
                    .append(errorString)
                    .c_str());
        }

        AZ_Error(
            __func__,
            false,
            std::string("RGL encountered an error ")
                .append(errorLocationString.c_str())
                .append(" with message: ")
                .append(errorString)
                .c_str());

        std::cout << std::string("RGL encountered an error ")
                         .append(errorLocationString.c_str())
                         .append(" with message: ")
                         .append(errorString)
                         .c_str()
                  << std::endl;

        if (successDest)
        {
            *successDest = false;
        }
    }

    rgl_mat3x4f RglMat3x4FromAzMatrix3x4(const AZ::Matrix3x4& azMatrix)
    {
        return {
            {
                { azMatrix.GetRow(0).GetX(), azMatrix.GetRow(0).GetY(), azMatrix.GetRow(0).GetZ(), azMatrix.GetRow(0).GetW() },
                { azMatrix.GetRow(1).GetX(), azMatrix.GetRow(1).GetY(), azMatrix.GetRow(1).GetZ(), azMatrix.GetRow(1).GetW() },
                { azMatrix.GetRow(2).GetX(), azMatrix.GetRow(2).GetY(), azMatrix.GetRow(2).GetZ(), azMatrix.GetRow(2).GetW() },
            },
        };
    }

    AZ::Matrix3x4 AzMatrix3x4FromRglMat3x4(const rgl_mat3x4f& rglMatrix)
    {
        const float m_rowMajorValues[]{
            rglMatrix.value[0][0], rglMatrix.value[0][1], rglMatrix.value[0][2], rglMatrix.value[0][3],
            rglMatrix.value[1][0], rglMatrix.value[1][1], rglMatrix.value[1][2], rglMatrix.value[1][3],
            rglMatrix.value[2][0], rglMatrix.value[2][1], rglMatrix.value[2][2], rglMatrix.value[2][3],
        };
        return AZ::Matrix3x4::CreateFromRowMajorFloat12(m_rowMajorValues);
    }

    AZ::Vector3 AzVector3FromRglVec3f(const rgl_vec3f& rglVector)
    {
        return { rglVector.value[0], rglVector.value[1], rglVector.value[2] };
    }

    rgl_vec3f RglVector3FromAzVec3f(const AZ::Vector3& azVector)
    {
        return { azVector.GetX(), azVector.GetY(), azVector.GetZ() };
    }

    rgl_vec2f RglVec2fFromAzVector2(const AZ::Vector2& azVector)
    {
        return { azVector.GetX(), azVector.GetY() };
    }
} // namespace RGL::Utils
