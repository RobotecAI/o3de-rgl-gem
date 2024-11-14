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
set(FILES
        Source/Entity/ActorEntityManager.cpp
        Source/Entity/ActorEntityManager.h
        Source/Entity/MeshEntityManager.cpp
        Source/Entity/MeshEntityManager.h
        Source/Entity/EntityManager.cpp
        Source/Entity/EntityManager.h
        Source/Entity/EntityTagListener.cpp
        Source/Entity/EntityTagListener.h
        Source/Entity/MaterialEntityManager.cpp
        Source/Entity/MaterialEntityManager.h
        Source/Entity/Terrain/TerrainData.cpp
        Source/Entity/Terrain/TerrainData.h
        Source/Entity/Terrain/TerrainEntityManagerSystemComponent.cpp
        Source/Entity/Terrain/TerrainEntityManagerSystemComponent.h
        Source/Lidar/LidarRaycaster.cpp
        Source/Lidar/LidarRaycaster.h
        Source/Lidar/LidarSystem.cpp
        Source/Lidar/LidarSystem.h
        Source/Lidar/LidarSystemNotificationBus.h
        Source/Lidar/PipelineGraph.cpp
        Source/Lidar/PipelineGraph.h
        Source/Model/ModelLibraryBus.h
        Source/Model/ModelLibrary.cpp
        Source/Model/ModelLibrary.h
        Source/RGLSystemComponent.cpp
        Source/RGLSystemComponent.h
        Source/Utilities/BlockCompression.cpp
        Source/Utilities/BlockCompression.h
        Source/Utilities/RGLUtils.cpp
        Source/Utilities/RGLUtils.h
        Source/Wrappers/RglEntity.cpp
        Source/Wrappers/RglEntity.h
        Source/Wrappers/RglMesh.cpp
        Source/Wrappers/RglMesh.h
        Source/Wrappers/RglTexture.cpp
        Source/Wrappers/RglTexture.h
        Source/SceneConfiguration.cpp
        Source/SceneConfigurationComponent.cpp
        Source/SceneConfigurationComponent.h
)
