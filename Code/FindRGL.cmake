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
set(RGL_VERSION 0.18.0)
set(RGL_TAG v${RGL_VERSION})

set(RGL_LINUX_ZIP_FILENAME_BASE RGL-full-linux-x64)
set(RGL_LINUX_ZIP_FILENAME ${RGL_LINUX_ZIP_FILENAME_BASE}.zip)

set(RGL_LINUX_ZIP_URL https://github.com/RobotecAI/RobotecGPULidar/releases/download/${RGL_TAG}/${RGL_LINUX_ZIP_FILENAME})
set(RGL_SRC_ZIP_URL https://github.com/RobotecAI/RobotecGPULidar/archive/refs/tags/${RGL_TAG}.zip)

set(RGL_SRC_ZIP_FILENAME_BASE RobotecGPULidar-${RGL_VERSION})
set(RGL_SRC_ZIP_FILENAME ${RGL_SRC_ZIP_FILENAME_BASE}.zip)

set(DEST_SO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/RobotecGPULidar)
set(DEST_API_DIR ${DEST_SO_DIR}/include/rgl/api)

set(SO_FILENAME libRobotecGPULidar.so)
set(API_FILENAME core.h)
set(API_EXT_ROS_FILENAME ros2.h)

# Paths relative to the .zip file root.
set(SO_REL_PATH ${SO_FILENAME})

set(SRC_ROOT_REL_PATH RobotecGPULidar-${RGL_VERSION})

set(CORE_API_REL_PATH ${SRC_ROOT_REL_PATH}/include/rgl/api)
set(ROS2_API_REL_PATH ${SRC_ROOT_REL_PATH}/extensions/ros2/include/rgl/api)

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# This check is performed to mitigate Clion multi-profile project reload issues
# (each profile would execute the file download, extraction and removal without it).
# Note: This check does not provide a full assurance (not atomic) but is good enough
#       since this is a Clion-specific issue.
if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/DOWNLOAD_RGL)
    FILE(TOUCH ${CMAKE_CURRENT_SOURCE_DIR}/DOWNLOAD_RGL)

    # Download the RGL archive files
    file(DOWNLOAD
            ${RGL_LINUX_ZIP_URL}
            ${DEST_SO_DIR}/${RGL_LINUX_ZIP_FILENAME}
            )

    file(DOWNLOAD
            ${RGL_SRC_ZIP_URL}
            ${DEST_API_DIR}/${RGL_SRC_ZIP_FILENAME}
            )

    # Extract the contents of the downloaded archive files
    file(ARCHIVE_EXTRACT INPUT ${DEST_SO_DIR}/${RGL_LINUX_ZIP_FILENAME}
            DESTINATION ${DEST_SO_DIR}
            PATTERNS ${SO_REL_PATH}
            VERBOSE
            )

    file(ARCHIVE_EXTRACT INPUT ${DEST_API_DIR}/${RGL_SRC_ZIP_FILENAME}
            DESTINATION ${DEST_API_DIR}
            PATTERNS ${CORE_API_REL_PATH}/* ${ROS2_API_REL_PATH}/*
            VERBOSE
            )

    # Move the extracted files to their desired locations
    file(RENAME ${DEST_SO_DIR}/${SO_REL_PATH} ${DEST_SO_DIR}/${SO_FILENAME})
    file(COPY ${DEST_API_DIR}/${CORE_API_REL_PATH} DESTINATION ${DEST_SO_DIR}/include/rgl/)
    file(COPY ${DEST_API_DIR}/${ROS2_API_REL_PATH} DESTINATION ${DEST_SO_DIR}/include/rgl/)

    # Remove the unwanted byproducts
    file(REMOVE_RECURSE ${DEST_API_DIR}/${RGL_SRC_ZIP_FILENAME_BASE})
    file(REMOVE ${DEST_API_DIR}/${RGL_SRC_ZIP_FILENAME})
    file(REMOVE ${DEST_SO_DIR}/${RGL_LINUX_ZIP_FILENAME})

    file(REMOVE ${CMAKE_CURRENT_SOURCE_DIR}/DOWNLOAD_RGL)
else ()
    message(WARNING "Omitting the RobotecGPULidar library download. This is intended when using the Clion multi-profile"
    " project reload. This may also happen due to interruption of previous project configurations. If you have"
    " any issues related to the libRobotecGPULidar.so file please make sure that the DOWNLOAD_RGL file is not"
    " present under the Code directory."
    )
endif ()

# Paths used by external targets
set(RGL_SO_DIR ${DEST_SO_DIR}/${SO_FILENAME})
set(RGL_INCLUDE_DIR ${DEST_SO_DIR}/include)
