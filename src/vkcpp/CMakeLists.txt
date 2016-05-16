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

set(VKCPP_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(VKCPP_INCLUDE_DIR ${VKCPP_DIR}/include)
set(VKCPP_HEADER_DIR ${VKCPP_INCLUDE_DIR}/vkcpp)
set(VKCPP_SRC_DIR ${VKCPP_DIR}/src)
set(VKCPP_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/vkcpp)

find_package(PythonInterp REQUIRED)

# Check for Jinja2
message(STATUS "${PYTHON_EXECUTABLE}")
execute_process(
    COMMAND ${PYTHON_EXECUTABLE} -c "import jinja2"
    RESULT_VARIABLE RET
)
if (NOT RET EQUAL 0)
    message(FATAL_ERROR "Missing dependencies for VkCPP generation, please ensure you have python-jinja2 installed.")
endif()

set(VKCPP_COMMAND
    ${PYTHON_EXECUTABLE} ${VKCPP_DIR}/generate.py ${VKCPP_DIR}/vk.xml
    -e ${VKCPP_DIR}/ExtensionList.txt
    -t ${VKCPP_DIR}/templates
    -s ${VKCPP_DIR}/sources
    -o ${VKCPP_OUTPUT_DIR}
)

execute_process(
    COMMAND ${VKCPP_COMMAND} --print-dependencies
    OUTPUT_VARIABLE VKCPP_DEPENDENCIES
    RESULT_VARIABLE RET
)
if (NOT RET EQUAL 0)
    message(FATAL_ERROR "Failed to get the VkCPP dependencies")
endif()

execute_process(
    COMMAND ${VKCPP_COMMAND} --print-outputs
    OUTPUT_VARIABLE VKCPP_OUTPUTS
)
if (NOT RET EQUAL 0)
    message(FATAL_ERROR "Failed to get the VkCPP outputs")
endif()

add_custom_command(
    OUTPUT ${VKCPP_OUTPUTS}
    COMMAND ${VKCPP_COMMAND}
    DEPENDS ${VKCPP_DEPENDENCIES} ${VKCPP_DIR}/generate.py ${VKCPP_DIR}/vk.xml ${VKCPP_DIR}/ExtensionList.txt
    COMMENT "Generating the VkCPP files."
)
if (NOT RET EQUAL 0)
    message(FATAL_ERROR "Failed to generate the VkCPP files")
endif()

add_library(vkcpp STATIC
    ${VKCPP_HEADER_DIR}/EnumClassBitmasks.h
    ${VKCPP_HEADER_DIR}/FunctionLoader.h
    ${VKCPP_HEADER_DIR}/LoaderManager.h
    ${VKCPP_HEADER_DIR}/vk_platform.h
    ${VKCPP_SRC_DIR}/FunctionLoader.cpp
    ${VKCPP_SRC_DIR}/LoaderManager.cpp
    ${VKCPP_OUTPUTS}
)
target_include_directories(vkcpp SYSTEM PRIVATE ${VKCPP_DIR}/external/vulkan/include)
target_include_directories(vkcpp PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(vkcpp PUBLIC ${VKCPP_DIR}/include)

add_executable(vkcpp_unittests
    ${VKCPP_DIR}/tests/BitmaskTests.cpp
    ${VKCPP_DIR}/tests/LoaderTests.cpp
    ${VKCPP_DIR}/tests/MockGetProc.cpp
    ${VKCPP_DIR}/tests/MockGetProc.h
    ${VKCPP_DIR}/tests/VkCppTestsMain.cpp
)
target_link_libraries(vkcpp_unittests vkcpp gtest)

set_target_properties(vkcpp vkcpp_unittests PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
)
