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

#include "VkContext.h"

#include "VkSwapchain.h"

#include <GLFW/glfw3.h>
#include <vkcpp/GLFW.h>

namespace Renderer {
namespace Vulkan {

    // Helper Vulkan functions
    namespace {
        bool GetInstanceInfo(const FunctionPointers& vk, InstanceInfo* info) {
            uint32_t layerCount = 0;
            VK_TRY(vk.EnumerateInstanceLayerProperties(&layerCount, nullptr));
            info->layers.resize(layerCount);
            VK_TRY(vk.EnumerateInstanceLayerProperties(&layerCount, info->layers.data()));

            uint32_t extensionCount = 0;
            VK_TRY(vk.EnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr));
            info->extensions.resize(extensionCount);
            VK_TRY(vk.EnumerateInstanceExtensionProperties(nullptr, &extensionCount, info->extensions.data()));

            return true;
        }


        bool GetPhysicalDevices(const FunctionPointers& vk, vk::Instance instance, std::vector<vk::PhysicalDevice>* devices) {
            uint32_t numPhysicalDevices = 0;
            VK_TRY(vk.EnumeratePhysicalDevices(instance, &numPhysicalDevices, nullptr));
            devices->resize(numPhysicalDevices);
            VK_TRY(vk.EnumeratePhysicalDevices(instance, &numPhysicalDevices, devices->data()));

            return true;
        }

        bool GetDeviceInfo(const FunctionPointers& vk, vk::PhysicalDevice device, DeviceInfo* info) {
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

            return true;
        }
    }

    FunctionPointers::FunctionPointers():
        vk::LoaderManager(reinterpret_cast<vk::UntypedFnptr>(vk::GLFW::GetInstanceProcAddress)),
        vk::VulkanLoader(this),
        vk::KHRDisplayLoader(this),
        vk::KHRSurfaceLoader(this),
        vk::KHRSwapchainLoader(this)
    {
    }

    Context::Context() {
        swapchain = new Swapchain(this);
    }

    Context::~Context() {
        // TODO(kangz) WaitIdle and destroy things in the GlobalInfo

        delete swapchain;
    }

    bool Context::Initialize(GLFWwindow* window, std::string applicationName, uint32_t applicationVersion) {
        if (!InitializeGlobalInfo(applicationName, applicationVersion)) {
            return false;
        }

        if (!swapchain->Initialize(window)) {
            return false;
        }

        return true;
    }

    bool Context::InitializeGlobalInfo(std::string applicationName, uint32_t applicationVersion) {
        if (!glfwVulkanSupported()) {
            Log::Warn("GLFW didn't find Vulkan support");
            return false;
        }

        vk.LoadGlobals();

        // Get the instance info
        {
            if (!GetInstanceInfo(vk, &info.instanceInfo)) {
                return false;
            }

            Log::Debug("Global layers:");
            for (const auto& layer : info.instanceInfo.layers) {
                Log::Debug(" - %s", layer.layerName);
            }

            Log::Debug("Global extensions:");
            for (const auto& extension : info.instanceInfo.extensions) {
                Log::Debug(" - %s", extension.extensionName);
            }
        }

        // Create the instance
        auto& instance = info.instance;
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

                for (const auto& extension : swapchain->GetRequiredGlobalExtensions()) {
                    required.insert(extension);
                }

                for (const auto& extension : info.instanceInfo.extensions) {
                    available.insert(extension.extensionName);
                }

                optional.insert("VK_EXT_debug_report");
            }

            std::vector<const char*> extensionPointers;
            {
                for (const auto& extension : required) {
                    if (available.count(extension) == 0) {
                        Log::Warn("Required Vulkan extension %s not available.", extension);
                        return false;
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
                .pApplicationName = applicationName.c_str(),
                .applicationVersion = applicationVersion,
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
            VK_TRY(vk.CreateInstance(&instanceInfo, nullptr, &info.instance));
        }

        vk.SetInstance(info.instance);

        // Get physical device info
        {
            GetPhysicalDevices(vk, instance, &info.physicalDevices);

            Log::Debug("Physical devices:");
            for (const auto& physicalDevice : info.physicalDevices) {
                DeviceInfo deviceInfo;
                if (!GetDeviceInfo(vk, physicalDevice, &deviceInfo)) {
                    return false;
                }

                Log::Debug(" - %s", deviceInfo.properties.deviceName);

                info.physicalDevicesInfo.push_back(deviceInfo);
            }
        }

        info.physicalDevice = info.physicalDevices[0];

        {
            if (!GetDeviceInfo(vk, info.physicalDevice, &info.deviceInfo)) {
                return false;
            }

            Log::Debug("Device layers:");
            for (const auto& layer : info.deviceInfo.layers) {
                Log::Debug(" - %s", layer.layerName);
            }

            Log::Debug("Device extensions:");
            for (const auto& extension : info.deviceInfo.extensions) {
                Log::Debug(" - %s", extension.extensionName);
            }
        }

        // Choose the queue families
        {
            bool hasGraphics = false;
            bool hasPresent = false;
            for (size_t i = 0; i < info.deviceInfo.queueFamilies.size(); ++i) {
                auto flags = info.deviceInfo.queueFamilies[i].queueFlags;

                auto kGraphicsFlags = vk::QueueFlags::Graphics | vk::QueueFlags::Compute | vk::QueueFlags::Transfer;
                if (!hasGraphics && (flags & kGraphicsFlags)) {
                    info.graphicsQueueFamily = i;
                    hasGraphics = true;
                }

                if (!hasPresent && vk::GLFW::GetPhysicalDevicePresentationSupport(info.instance, info.physicalDevice, i)) {
                    info.presentQueueFamily = i;
                    hasPresent = true;
                }
            }

            if (!hasGraphics || !hasPresent) {
                return false;
            }
        }

        // Create the logical device
        {
            const float kOnes[] = {1.0f, 1.0f};

            std::vector<vk::DeviceQueueCreateInfo> queueFamilies;
            {
                vk::DeviceQueueCreateInfo familyInfo = {
                    .queueFamilyIndex = info.graphicsQueueFamily,
                    .queueCount = (info.graphicsQueueFamily == info.presentQueueFamily) ? uint32_t(2) : uint32_t(1),
                    .pQueuePriorities = kOnes
                };
                queueFamilies.push_back(familyInfo);
            }

            if (info.graphicsQueueFamily != info.presentQueueFamily) {
                vk::DeviceQueueCreateInfo familyInfo = {
                    .queueFamilyIndex = info.presentQueueFamily,
                    .queueCount = 1,
                    .pQueuePriorities = kOnes
                };
                queueFamilies.push_back(familyInfo);
            }

            // TODO(kangz) find a better way to ask part of the renderer what extensions they would like
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

            VK_TRY(vk.CreateDevice(info.physicalDevice, &deviceInfo, nullptr, &info.device));
        }

        // Gather the queues
        {
            vk.GetDeviceQueue(info.device, info.graphicsQueueFamily, 0, &info.graphicsQueue);

            uint32_t presentOffset = (info.graphicsQueueFamily == info.presentQueueFamily) ? 1 : 0;
            vk.GetDeviceQueue(info.device, info.presentQueueFamily, presentOffset, &info.presentQueue);
        }

        return true;
    }

    const GlobalInfo& Context::GetGlobalInfo() const {
        return info;
    }

    const FunctionPointers& Context::GetFunctionPointers() const {
        return vk;
    }
}
}
