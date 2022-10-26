# Copyright (c) Contributors to the Open 3D Engine Project.
# For complete copyright and license terms please see the LICENSE at the root of this distribution.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT

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
        PRIVATE
            Source
    BUILD_DEPENDENCIES
        PUBLIC
            AZ::AzCore
            AZ::AzFramework
            AZ::AzToolsFramework
            Gem::Atom_RPI.Public
            Gem::AtomLyIntegration_CommonFeatures.Static
            Gem::ROS2.Static
)

target_compile_options(RobotecGPULidar PUBLIC "$<$<COMPILE_LANGUAGE:CXX>:-fexceptions;-Wno-error>")
target_compile_options(spdlog PUBLIC "$<$<COMPILE_LANGUAGE:CXX>:-fexceptions>")

target_link_libraries(RGL.Static PUBLIC RobotecGPULidar)

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
