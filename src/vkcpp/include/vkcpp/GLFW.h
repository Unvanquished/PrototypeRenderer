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

#ifndef VKCPP_GLFW_H_
#define VKCPP_GLFW_H_

// Header for the VkCPP <-> GLFW compatibility functions. You should include GLFW before including this header.

#include "vkcpp/Vulkan.h"
#include "vkcpp/KHRSurface.h"

namespace vk {

    namespace GLFW {

        vk::UntypedFnptr GetInstanceProcAddress(vk::Instance instance, const char* procName);
        int GetPhysicalDevicePresentationSupport(vk::Instance instance, vk::PhysicalDevice device, uint32_t queueFamily);
        vk::Result CreateWindowSurface(vk::Instance instance, GLFWwindow* window, const vk::AllocationCallbacks* allocator, vk::SurfaceKHR* surface);

    }

}

#endif // VKCPP_GLFW_H_
