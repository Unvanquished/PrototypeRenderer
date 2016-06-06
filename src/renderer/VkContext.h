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

#ifndef RENDERER_VKCONTEXT_H_
#define RENDERER_VKCONTEXT_H_

#include <vkcpp/EXTDebugReport.h>
#include <vkcpp/LoaderManager.h>
#include <vkcpp/Vulkan.h>

struct GLFWwindow;

namespace Renderer {
namespace Vulkan {


    #define VK_TRY(expression) \
        { \
            vk::Result result = (expression); \
            if (result != vk::Result::Success) {return false;} \
        }

    struct InstanceInfo {
        std::vector<vk::LayerProperties> layers;
        std::vector<vk::ExtensionProperties> extensions;
    };

    struct DeviceInfo {
        std::vector<vk::LayerProperties> layers;
        std::vector<vk::ExtensionProperties> extensions;
        std::vector<vk::QueueFamilyProperties> queueFamilies;
        vk::PhysicalDeviceProperties properties;
        vk::PhysicalDeviceMemoryProperties memory;
    };

    struct GlobalInfo {
        InstanceInfo instanceInfo;
        vk::Instance instance;

        std::vector<vk::PhysicalDevice> physicalDevices;
        std::vector<DeviceInfo> physicalDevicesInfo;

        vk::PhysicalDevice physicalDevice;
        DeviceInfo deviceInfo;
        vk::Device device;

        uint32_t graphicsQueueFamily;
        vk::Queue graphicsQueue;
        uint32_t presentQueueFamily;
        vk::Queue presentQueue;
    };

    class FunctionPointers :
        public vk::LoaderManager,
        public vk::VulkanLoader {
        public:
            FunctionPointers();
    };

    class Context {
        public:
            Context();
            ~Context();

            bool Initialize(GLFWwindow* window, std::string applicationName, uint32_t applicationVersion);

            const GlobalInfo& GetGlobalInfo() const;
            const FunctionPointers& GetFunctionPointers() const;

        private:
            bool InitializeGlobalInfo(std::string applicationName, uint32_t applicationVersion);
            GlobalInfo info;
            FunctionPointers vk;
    };

}
}

#endif // RENDERER_VKCONTEXT_H_
