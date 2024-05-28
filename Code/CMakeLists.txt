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

include(FindRGL.cmake)
ly_add_target(
    NAME RGL.Static STATIC
    NAMESPACE Gem
    FILES_CMAKE
        rgl_header_files.cmake
        rgl_files.cmake
    PLATFORM_INCLUDE_FILES
        ${CMAKE_CURRENT_LIST_DIR}/Platform/Common/${PAL_TRAIT_COMPILER_ID}/rgl_static_${PAL_TRAIT_COMPILER_ID_LOWERCASE}.cmake
    INCLUDE_DIRECTORIES
        PUBLIC
            Include
            ${RGL_INCLUDE_DIR}
        PRIVATE
            Source
    BUILD_DEPENDENCIES
        PUBLIC
            AZ::AzCore
            AZ::AzFramework
            Gem::Atom_RPI.Public
            Gem::AtomLyIntegration_CommonFeatures.Static
            Gem::ROS2.Static
            Gem::EMotionFX.Static
            Gem::LmbrCentral
)

ly_target_link_libraries(RGL.Static PUBLIC ${RGL_SO_DIR})

ly_add_target_files(
        TARGETS
        RGL.Static
        FILES
        ${RGL_SO_DIR}
)

ly_add_target(
    NAME RGL.API HEADERONLY
    NAMESPACE Gem
    FILES_CMAKE
        rgl_header_files.cmake
    INCLUDE_DIRECTORIES
        INTERFACE
            Include
)

# Here add RGL target, it depends on the RGL.Static
ly_add_target(
    NAME RGL ${PAL_TRAIT_MONOLITHIC_DRIVEN_MODULE_TYPE}
    NAMESPACE Gem
    FILES_CMAKE
        rgl_shared_files.cmake
    INCLUDE_DIRECTORIES
        PUBLIC
            Include
        PRIVATE
            Source
    BUILD_DEPENDENCIES
        PRIVATE
            Gem::RGL.Static
)

# By default, we will specify that the above target RGL would be used by
# Client and Server type targets when this gem is enabled.  If you don't want it
# active in Clients or Servers by default, delete one of both of the following lines:
ly_create_alias(NAME RGL.Clients NAMESPACE Gem TARGETS Gem::RGL)
ly_create_alias(NAME RGL.Servers NAMESPACE Gem TARGETS Gem::RGL)

if(PAL_TRAIT_BUILD_HOST_TOOLS)
    ly_add_target(
        NAME RGL.Editor.Static STATIC
        NAMESPACE Gem
        FILES_CMAKE
            rgl_editor_files.cmake
        PLATFORM_INCLUDE_FILES
            ${CMAKE_CURRENT_LIST_DIR}/Platform/Common/${PAL_TRAIT_COMPILER_ID}/rgl_static_${PAL_TRAIT_COMPILER_ID_LOWERCASE}.cmake
        INCLUDE_DIRECTORIES
            PRIVATE
                Source
            PUBLIC
                Include
        BUILD_DEPENDENCIES
            PUBLIC
                AZ::AzToolsFramework
                Gem::RGL.Static
                Gem::LmbrCentral.Static
    )

    ly_add_target(
        NAME RGL.Editor GEM_MODULE
        NAMESPACE Gem
        AUTOMOC
        FILES_CMAKE
            rgl_editor_shared_files.cmake
        INCLUDE_DIRECTORIES
            PRIVATE
                Source
            PUBLIC
                Include
        BUILD_DEPENDENCIES
            PUBLIC
                Gem::RGL.Editor.Static
    )

    # By default, we will specify that the above target RGL would be used by
    # Tool and Builder type targets when this gem is enabled.  If you don't want it
    # active in Tools or Builders by default, delete one of both of the following lines:
    ly_create_alias(NAME RGL.Tools    NAMESPACE Gem TARGETS Gem::RGL.Editor)
    ly_create_alias(NAME RGL.Builders NAMESPACE Gem TARGETS Gem::RGL.Editor)
endif()
