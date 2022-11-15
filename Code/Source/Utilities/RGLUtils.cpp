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
    void RglUtils::ErrorCheck(const rgl_status_t& status)
    {
        static const AZStd::unordered_set<rgl_status_t> InvalidStates = { RGL_INVALID_STATE, RGL_INTERNAL_EXCEPTION, RGL_INVALID_PIPELINE };

        if (status == 0)
        {
            return;
        }

        const char* errorString;
        rgl_get_last_error_string(&errorString);
        if (InvalidStates.contains(status))
        {
            AZ_Assert(false, std::string("RGL encountered an unrecoverable error with message: ").append(errorString).c_str())
        }

        AZ_Error("ErrorCheck", false, std::string("RGL encountered an error with message: ").append(errorString).c_str())
    }
}; // namespace RGL
