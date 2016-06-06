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

#include "Renderer.h"

#include <vkcpp/EXTDebugReport.h>
#include <vkcpp/LoaderManager.h>
#include <vkcpp/Vulkan.h>

#include <GLFW/glfw3.h>
#include <vkcpp/GLFW.h>

#include "common/Common.h"
#include "framework/System.h"

namespace Renderer {

    class VulkanFunctionPointers :
        public vk::LoaderManager,
        public vk::VulkanLoader {
        public:
            VulkanFunctionPointers():
                vk::LoaderManager(reinterpret_cast<vk::UntypedFnptr>(vk::GLFW::GetInstanceProcAddress)),
                vk::VulkanLoader(this)
            {
            }
    };

    namespace {

        #define VK_TRY(expression) \
            { \
                vk::Result result = (expression); \
                if (result != vk::Result::Success) {return result;}; \
            }

        struct InstanceInfo {
            std::vector<vk::LayerProperties> layers;
            std::vector<vk::ExtensionProperties> extensions;
        };

        vk::Result GetInstanceInfo(const VulkanFunctionPointers& vk, InstanceInfo* info) {
            uint32_t layerCount = 0;
            VK_TRY(vk.EnumerateInstanceLayerProperties(&layerCount, nullptr));
            info->layers.resize(layerCount);
            VK_TRY(vk.EnumerateInstanceLayerProperties(&layerCount, info->layers.data()));

            uint32_t extensionCount = 0;
            VK_TRY(vk.EnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr));
            info->extensions.resize(extensionCount);
            VK_TRY(vk.EnumerateInstanceExtensionProperties(nullptr, &extensionCount, info->extensions.data()));

            return vk::Result::Success;
        }

        vk::Result GetPhysicalDevices(const VulkanFunctionPointers& vk, vk::Instance instance, std::vector<vk::PhysicalDevice>* devices) {
            uint32_t numPhysicalDevices = 0;
            VK_TRY(vk.EnumeratePhysicalDevices(instance, &numPhysicalDevices, nullptr));
            devices->resize(numPhysicalDevices);
            VK_TRY(vk.EnumeratePhysicalDevices(instance, &numPhysicalDevices, devices->data()));

            return vk::Result::Success;
        }

        struct DeviceInfo {
            std::vector<vk::LayerProperties> layers;
            std::vector<vk::ExtensionProperties> extensions;
            std::vector<vk::QueueFamilyProperties> queueFamilies;
            vk::PhysicalDeviceProperties properties;
            vk::PhysicalDeviceMemoryProperties memory;
        };

        vk::Result GetDeviceInfo(const VulkanFunctionPointers& vk, vk::PhysicalDevice device, DeviceInfo* info) {
            uint32_t layerCount = 0;
            VK_TRY(vk.EnumerateDeviceLayerProperties(device, &layerCount, nullptr));
            info->layers.resize(layerCount);
            VK_TRY(vk.EnumerateDeviceLayerProperties(device, &layerCount, info->layers.data()));

            uint32_t extensionCount = 0;
            VK_TRY(vk.EnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr));
            info->extensions.resize(extensionCount);
            VK_TRY(vk.EnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, info->extensions.data()));

            uint32_t familyCount = 0;
            vk.GetPhysicalDeviceQueueFamilyProperties(device, &familyCount, nullptr);
            info->queueFamilies.resize(familyCount);
            vk.GetPhysicalDeviceQueueFamilyProperties(device, &familyCount, info->queueFamilies.data());

            vk.GetPhysicalDeviceMemoryProperties(device, &info->memory);
            vk.GetPhysicalDeviceProperties(device, &info->properties);

            return vk::Result::Success;
        }

    }

    Renderer::Renderer() {
    }

    void Renderer::Initialize(GLFWwindow* window) {
        vk::Result result;
        if (!glfwVulkanSupported()) {
            Sys::Error("GLFW didn't find Vulkan support");
        }

        VulkanFunctionPointers vk;
        vk.LoadGlobals();

        // Get the instance info
        InstanceInfo instanceInfo;
        {
            result = GetInstanceInfo(vk, &instanceInfo);
            ASSERT(result == vk::Result::Success);

            Log::Debug("Global layers:");
            for (const auto& layer : instanceInfo.layers) {
                Log::Debug(" - %s", layer.layerName);
            }

            Log::Debug("Global extensions:");
            for (const auto& extension : instanceInfo.extensions) {
                Log::Debug(" - %s", extension.extensionName);
            }
        }

        // Create the instance
        vk::Instance instance;
        {
            using ExtensionSet = std::unordered_set<std::string>;

            ExtensionSet available, required, optional;
            {
                int count = 0;
                const char** glfwRequiredExtensions = glfwGetRequiredInstanceExtensions(&count);
                ASSERT(glfwRequiredExtensions != nullptr);

                while (count --> 0) {
                    required.insert(glfwRequiredExtensions[count]);
                }

                for (const auto& extension : instanceInfo.extensions) {
                    available.insert(extension.extensionName);
                }

                optional.insert("VK_EXT_debug_report");
            }

            std::vector<const char*> extensionPointers;
            {
                for (const auto& extension : required) {
                    if (available.count(extension) == 0) {
                        Sys::Error("Required Vulkan extension %s not available.", extension);
                    }
                    extensionPointers.push_back(extension.c_str());
                }

                for (const auto& extension : optional) {
                    if (available.count(extension) == 0) {
                        Log::Notice("NOT using Vulkan extension %s", extension);
                    } else {
                        Log::Notice("Using Vulkan extension %s", extension);
                        extensionPointers.push_back(extension.c_str());
                    }
                }
            }

            const vk::ApplicationInfo appInfo = {
                .pApplicationName = "PrototypeRenderer",
                .applicationVersion = 0,
                .pEngineName = "PrototypeRenderer",
                .engineVersion = 0,
                //TODO(kangz) add the version macro in vkcpp
                .apiVersion = (1 << 22) + 3
            };
            const vk::InstanceCreateInfo instanceInfo = {
                .pApplicationInfo = &appInfo,
                .enabledExtensionCount = static_cast<uint32_t>(extensionPointers.size()),
                .ppEnabledExtensionNames = extensionPointers.data(),
            };
            result = vk.CreateInstance(&instanceInfo, nullptr, &instance);
            ASSERT(result == vk::Result::Success);
        }

        vk.SetInstance(instance);

        std::vector<vk::PhysicalDevice> physicalDevices;
        {
            GetPhysicalDevices(vk, instance, &physicalDevices);

            Log::Debug("Physical devices:");
            for (const auto& physicalDevice : physicalDevices) {
                DeviceInfo info;
                result = GetDeviceInfo(vk, physicalDevice, &info);
                ASSERT(result == vk::Result::Success);

                Log::Debug(" - %s", info.properties.deviceName);
            }
        }

        vk::PhysicalDevice physicalDevice = physicalDevices[0];

        DeviceInfo deviceInfo;
        {
            result = GetDeviceInfo(vk, physicalDevice, &deviceInfo);
            ASSERT(result == vk::Result::Success);

            Log::Debug("Device layers:");
            for (const auto& layer : deviceInfo.layers) {
                Log::Debug(" - %s", layer.layerName);
            }

            Log::Debug("Device extensions:");
            for (const auto& extension : deviceInfo.extensions) {
                Log::Debug(" - %s", extension.extensionName);
            }
        }

        uint32_t graphicsQueueFamily = 0;
        uint32_t presentQueueFamily = 0;
        {
            bool hasGraphics = false;
            bool hasPresent = false;
            for (size_t i = 0; i < deviceInfo.queueFamilies.size(); ++i) {
                auto flags = deviceInfo.queueFamilies[i].queueFlags;

                auto kGraphicsFlags = vk::QueueFlags::Graphics | vk::QueueFlags::Compute | vk::QueueFlags::Transfer;
                if (!hasGraphics && (flags & kGraphicsFlags)) {
                    graphicsQueueFamily = i;
                    hasGraphics = true;
                }

                if (!hasPresent && vk::GLFW::GetPhysicalDevicePresentationSupport(instance, physicalDevice, i)) {
                    presentQueueFamily = i;
                    hasPresent = true;
                }
            }

            ASSERT(hasGraphics && hasPresent);
        }

        vk::Device device;
        {
            const float kOnes[] = {1.0f, 1.0f};

            std::vector<vk::DeviceQueueCreateInfo> queueFamilies;

            {
                vk::DeviceQueueCreateInfo familyInfo = {
                    .queueFamilyIndex = graphicsQueueFamily,
                    .queueCount = (graphicsQueueFamily == presentQueueFamily) ? uint32_t(2) : uint32_t(1),
                    .pQueuePriorities = kOnes
                };
                queueFamilies.push_back(familyInfo);
            }

            if (graphicsQueueFamily != presentQueueFamily) {
                vk::DeviceQueueCreateInfo familyInfo = {
                    .queueFamilyIndex = presentQueueFamily,
                    .queueCount = 1,
                    .pQueuePriorities = kOnes
                };
                queueFamilies.push_back(familyInfo);
            }

            const char* kKHRSwapchain = "VK_KHR_swapchain";
            const vk::DeviceCreateInfo deviceInfo = {
                .queueCreateInfoCount = static_cast<uint32_t>(queueFamilies.size()),
                .pQueueCreateInfos = queueFamilies.data(),
                .enabledLayerCount = 0,
                .ppEnabledLayerNames = nullptr,
                .enabledExtensionCount = 1,
                .ppEnabledExtensionNames = &kKHRSwapchain,
                .pEnabledFeatures = nullptr
            };

            result = vk.CreateDevice(physicalDevice, &deviceInfo, nullptr, &device);
            ASSERT(result == vk::Result::Success);
        }

        vk::Queue graphicsQueue;
        vk::Queue presentQueue;
        {
            vk.GetDeviceQueue(device, graphicsQueueFamily, 0, &graphicsQueue);

            uint32_t presentOffset = (graphicsQueueFamily == presentQueueFamily) ? 1 : 0;
            vk.GetDeviceQueue(device, presentQueueFamily, presentOffset, &presentQueue);
        }

        exit(0);
    }

    void Renderer::Frame() {
    }

}
