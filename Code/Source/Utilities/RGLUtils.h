/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

#include "rgl/api/experimental.h"

namespace RGL
{
    //! Handles RGL - related errors. Each status returned by RGL API function passed to this function.
    void ErrorCheck(const rgl_status_t& status);
}; // namespace RGL