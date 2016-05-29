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

#include <vulkan/vulkan.h>

extern "C" {
    // Declaration required for GLFW.h to compile
    typedef struct GLFWwindow GLFWwindow;
}

#include "vkcpp/GLFW.h"

// Declarations copied from glfw3.h so that VkCPP doesn't have to know about GLFW a priori.
extern "C" {
    typedef void (*GLFWvkproc)(void);

    GLFWvkproc glfwGetInstanceProcAddress(VkInstance instance, const char* procname);
    int glfwGetPhysicalDevicePresentationSupport(VkInstance instance, VkPhysicalDevice device, uint32_t queuefamily);
    VkResult glfwCreateWindowSurface(VkInstance instance, GLFWwindow* window, const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface);
}

namespace vk {

    namespace GLFW {
        vk::UntypedFnptr GetInstanceProcAddress(vk::Instance instance, const char* procName) {
            return reinterpret_cast<vk::UntypedFnptr>(glfwGetInstanceProcAddress(reinterpret_cast<VkInstance>(instance), procName));
        }

        int GetPhysicalDevicePresentationSupport(vk::Instance instance, vk::PhysicalDevice device, uint32_t queueFamily) {
            return glfwGetPhysicalDevicePresentationSupport(reinterpret_cast<VkInstance>(instance), reinterpret_cast<VkPhysicalDevice>(device), queueFamily);
        }

        vk::Result CreateWindowSurface(vk::Instance instance, GLFWwindow* window, const vk::AllocationCallbacks* allocator, vk::SurfaceKHR* surface) { 
            return static_cast<vk::Result>(glfwCreateWindowSurface(reinterpret_cast<VkInstance>(instance), window, reinterpret_cast<const VkAllocationCallbacks*>(allocator), reinterpret_cast<VkSurfaceKHR*>(surface)));
        }
    }

}
