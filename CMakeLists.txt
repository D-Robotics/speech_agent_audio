# Copyright (c) 2024，D-Robotics.
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

cmake_minimum_required(VERSION 3.5)
project(speech_agent_audio)

# Default to C99
if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 99)
endif()

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

#if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
#  add_compile_options(-Wall -Wextra -Wpedantic)
#endif()

# find dependencies
find_package(ament_cmake REQUIRED)
# uncomment the following section in order to fill in
# further dependencies manually.
# find_package(<dependency> REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(audio_msg REQUIRED)
find_package(std_msgs REQUIRED)
find_package(jsoncpp REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
set(CMAKE_BUILD_TYPE "Release")

message("PREFIX_PATH: ${PREFIX_PATH}")

include_directories(include
  ${PROJECT_SOURCE_DIR}
)

set(SYS_ROOT ${CMAKE_SYSROOT})
message("SYS_ROOT is " ${SYS_ROOT})
include_directories(
  ${SYS_ROOT}/usr/include/
  ${SYS_ROOT}/usr/include/jsoncpp/
  ${PROJECT_SOURCE_DIR}/include/vtn/
  ${PROJECT_SOURCE_DIR}/include/vad/include/
)

link_directories(
  ${SYS_ROOT}/usr/lib/
  ${SYS_ROOT}/usr/lib/hobot/
  ${SYS_ROOT}/usr/hobot/lib/
  ${SYS_ROOT}/usr/lib/hobot/hobotlog
  ${PROJECT_SOURCE_DIR}/lib/
)

add_executable(${PROJECT_NAME}
  src/utils/alsa_device.cpp
  src/speech_agent_audio.cpp
  src/main.cpp
)

#set(BASE_LIBRARIES pthread dl rt m)
set(BASE_LIBRARIES pthread)
set(X3_AUDIO_LIB asound jsoncpp dl)

target_link_libraries(${PROJECT_NAME}
   ${X3_AUDIO_LIB} ${BASE_LIBRARIES}
   speech_agent_audio_bootstrap
   evad
)

ament_target_dependencies(${PROJECT_NAME}
"rclcpp"
"geometry_msgs"
"audio_msg")

target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

set(TAR_FILE "${CMAKE_SOURCE_DIR}/speech_agent_audio_v1.0.0.tar.gz")
set(EXTRACT_DIR "${CMAKE_INSTALL_PREFIX}/lib/")
file(MAKE_DIRECTORY ${EXTRACT_DIR})
if (NOT EXISTS "${TAR_FILE}")
  execute_process(
    COMMAND wget https://archive.d-robotics.cc/TogetheROS/files/speech_solution/speech_agent_audio/speech_agent_audio_v1.0.0.tar.gz
              -O ${TAR_FILE}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    ERROR_VARIABLE wget_error
    RESULT_VARIABLE wget_result
  )
  if(NOT wget_result EQUAL 0)
    message(FATAL_ERROR "Download failed: ${wget_error}")
  endif()
endif()

if (NOT IS_DIRECTORY "${EXTRACT_DIR}/speech_agent_audio")
  execute_process(
    COMMAND tar -zxvf ${TAR_FILE} -C ${EXTRACT_DIR}
    ERROR_VARIABLE tar_error
    RESULT_VARIABLE tar_result
  )
  if(NOT tar_result EQUAL 0)
    message(FATAL_ERROR "Extract failed: ${tar_error}")
  endif()

  file(REMOVE ${TAR_FILE})
endif()

# Install libraries
install(TARGETS ${PROJECT_NAME}
  DESTINATION lib/${PROJECT_NAME}/)

install(PROGRAMS
  ${PROJECT_SOURCE_DIR}/setup_env.sh
  DESTINATION share/${PROJECT_NAME}/)

install(DIRECTORY
${PROJECT_SOURCE_DIR}/launch/
DESTINATION share/${PROJECT_NAME}/launch)

# Causes the visibility macros to use dllexport rather than dllimport,
# which is appropriate when building the dll but not consuming it.
target_compile_definitions(${PROJECT_NAME}
  PRIVATE "RCLCPP_BUILDING_LIBRARY")

install(
  TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

# specific order: dependents before dependencies
ament_export_include_directories(include)
ament_export_libraries(${PROJECT_NAME})
ament_export_targets(${PROJECT_NAME})

ament_package()