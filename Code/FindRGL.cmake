# Copyright 2020-2021, Robotec.ai sp. z o.o.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
set(RGL_HASH e5ea8a1a74c253a304f4d1d4652391bca9d448e6)
set(RGL_TAG v0.11.3)

set(RGL_SO_URL https://github.com/RobotecAI/RobotecGPULidar/releases/download/${RGL_TAG}/libRobotecGPULidar.so)
set(RGL_API_URL https://raw.githubusercontent.com/RobotecAI/RobotecGPULidar/${RGL_HASH}/include/rgl/api/core.h)

set(RGL_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/RobotecGPULidar)
set(RGL_INCLUDE_DIR ${RGL_LIB_DIR}/include)

set(RGL_SO_PATH ${RGL_LIB_DIR}/libRobotecGPULidar.so)
set(RGL_API_PATH ${RGL_LIB_DIR}/include/rgl/api/core.h)

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

file(DOWNLOAD
        ${RGL_SO_URL}
        ${RGL_SO_PATH}
    )

file(DOWNLOAD
        ${RGL_API_URL}
        ${RGL_API_PATH}
    )