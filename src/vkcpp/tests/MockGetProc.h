// PrototypeRenderer Source Code
// Copyright (c) 2014-2016, Daemon Developers
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// * Neither the name of Daemon CBSE nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef VKCPP_TESTS_MOCK_GET_PROC_H_
#define VKCPP_TESTS_MOCK_GET_PROC_H_

#include <string>
#include <unordered_set>
#include <unordered_map>

#include "vkcpp/Vulkan.h"

// VkCpp tests should run without a Vulkan driver so we need to use a mock for the
// getProcAddress Vulkan functions. This mock allows a test to known which proc were
// queried and to use custom values for some procs.
struct MockGetProc {
    MockGetProc();
    ~MockGetProc();

    // Get the mock vkGetProcAddress to give to the a LoaderManager
    vk::UntypedFnptr GetVkGetProcAddress() const;
    vk::Instance GetInstance() const;

    // Set the value returned for a specific proc.
    void AddDefault(const char* name, vk::UntypedFnptr proc);

    // Function that will be used as the getProcAddress, tests needn't use it.
    vk::UntypedFnptr GetProc(vk::Instance instance, const char* name);

    std::unordered_set<std::string> queriedGlobalProcs;
    std::unordered_set<std::string> queriedInstanceProcs;

protected:
    vk::Instance instance;
    std::unordered_map<std::string, vk::UntypedFnptr> defaultProcs;
};

#endif // VKCPP_TESTS_MOCK_GET_PROC_H_
