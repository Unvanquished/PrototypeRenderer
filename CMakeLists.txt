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

add_subdirectory(${SRC_DIR}/vkcpp)

set(RENDERER_DIR ${SRC_DIR}/renderer)
set(RENDERER_SOURCES
    ${RENDERER_DIR}/Renderer.cpp
    ${RENDERER_DIR}/Renderer.h
    ${RENDERER_DIR}/VkContext.cpp
    ${RENDERER_DIR}/VkContext.h
    ${RENDERER_DIR}/VkSwapchain.cpp
    ${RENDERER_DIR}/VkSwapchain.h
)

set(APP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/app)
set(APP_SOURCES
    ${APP_DIR}/TestApplication.cpp
    ${RENDERER_SOURCES}
)
AddApplication(
    Target testapp
    ExecutableName testapp
    Libs glfw vkcpp
    Files ${APP_SOURCES}
)
target_include_directories(testapp PRIVATE ${SRC_DIR})
