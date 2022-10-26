# Copyright (c) Contributors to the Open 3D Engine Project.
# For complete copyright and license terms please see the LICENSE at the root of this distribution.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT

include(FetchContent)

FetchContent_Declare(
        RobotecGPULidar
        GIT_REPOSITORY git@github.com:RobotecAI/RobotecGPULidar.git
        GIT_TAG v0.10.2
)

FetchContent_MakeAvailable(RobotecGPULidar)
