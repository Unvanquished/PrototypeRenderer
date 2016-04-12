# Unvanquished BSD Source Code
# Copyright (c) 2013-2016, Unvanquished Developers
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#  * Neither the name of the <organization> nor the
#    names of its contributors may be used to endorse or promote products
#    derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

cmake_minimum_required(VERSION 2.8)
project(PrototypeRenderer C CXX)

set(DAEMON_EXTERNAL_APP ON)
set(Daemon_OUT ${CMAKE_CURRENT_BINARY_DIR})
add_subdirectory(daemon/daemon daemon_build)
include(DaemonFlags)

if(NOT MSVC)
    set_cxx_flag("-std=c++14")
endif()

# GLFW, only build the library
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/glfw)

# GoogleTest
set(GTEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/googletest/googletest)
add_library(gtest STATIC ${GTEST_DIR}/src/gtest-all.cc)
target_include_directories(gtest SYSTEM PUBLIC ${GTEST_DIR}/include)
target_include_directories(gtest PRIVATE ${GTEST_DIR})

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)

# VkCpp
set(VKCPP_DIR ${SRC_DIR}/vkcpp)
set(VKCPP_OUTPUT_DIR ${GENERATED_DIR}/vkcpp)

find_package(PythonInterp REQUIRED)

# Check for Jinja2
message(STATUS "${PYTHON_EXECUTABLE}")
execute_process(
    COMMAND ${PYTHON_EXECUTABLE} -c "import jinja2"
    RESULT_VARIABLE RET)
if (NOT RET EQUAL 0)
    message(FATAL_ERROR "Missing dependencies for VkCpp generation, please ensure you have python-jinja2 installed.")
endif()

set(VKCPP_ARGS ${PYTHON_EXECUTABLE} ${VKCPP_DIR}/generate.py -e ${VKCPP_DIR}/ExtensionList.txt -t ${VKCPP_DIR}/templates -s ${VKCPP_DIR}/sources -o ${VKCPP_OUTPUT_DIR})
execute_process(
    COMMAND ${VKCPP_ARGS} --print-dependencies ${VKCPP_DIR}/vk.xml
    OUTPUT_VARIABLE VKCPP_DEPENDENCIES
)
execute_process(
    COMMAND ${VKCPP_ARGS} --print-outputs ${VKCPP_DIR}/vk.xml
    OUTPUT_VARIABLE VKCPP_OUTPUTS
)
add_custom_command(
    OUTPUT ${VKCPP_OUTPUTS}
    COMMAND ${VKCPP_ARGS} ${VKCPP_DIR}/vk.xml
    DEPENDS ${VKCPP_DEPENDENCIES} ${VKCPP_DIR}/generate.py ${VKCPP_DIR}/vk.xml ${VKCPP_DIR}/ExtensionList.txt
    COMMENT "Generating the VkCPP files."
)
add_library(vkcpp STATIC ${VKCPP_OUTPUTS})
target_include_directories(vkcpp SYSTEM PRIVATE ${VKCPP_DIR}/external/vulkan/include)
target_include_directories(gtest PUBLIC ${GENERATED_DIR})

add_executable(vkcpp_unittests
    ${VKCPP_DIR}/tests/BitmaskTests.cpp
    ${VKCPP_DIR}/tests/LoaderTests.cpp
    ${VKCPP_DIR}/tests/MockGetProc.cpp
    ${VKCPP_DIR}/tests/MockGetProc.h
    ${VKCPP_DIR}/tests/VkCppTestsMain.cpp
)
target_link_libraries(vkcpp_unittests vkcpp gtest)

set(APP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/app)
AddApplication(
    Target testapp
    ExecutableName testapp
    Libs vkcpp
    Files ${APP_DIR}/TestApplication.cpp
)
