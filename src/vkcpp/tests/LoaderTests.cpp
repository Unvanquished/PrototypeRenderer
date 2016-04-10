// PrototypeRenderer BSD Source Code
// Copyright (c) 2013-2016, Daemon Developers
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the Daemon developers nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL DAEMON DEVELOPERS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "gtest/gtest.h"

#include "MockGetProc.h"
#include "vkcpp/LoaderManager.h"
#include "vkcpp/Vulkan.h"
#include "vkcpp/EXTDebugReport.h"

class LoaderTests: public ::testing::Test {
    public:
        LoaderTests()
        : manager(getProc.GetVkGetProcAddress()), vulkan(&manager), debugReport(&manager)
        {
        }

    protected:
        MockGetProc getProc;
        vk::LoaderManager manager;
        vk::VulkanLoader vulkan;
        vk::EXTDebugReportLoader debugReport;
};

TEST_F(LoaderTests, LoadGlobals) {
    manager.LoadGlobals();

    // Test that vkGetProcAddress is called exactly on the right functions (there are only 3 of them).
    ASSERT_TRUE(getProc.queriedInstanceProcs.empty());
    ASSERT_EQ(3, getProc.queriedGlobalProcs.size());
    ASSERT_EQ(1, getProc.queriedGlobalProcs.count("vkCreateInstance"));
    ASSERT_EQ(1, getProc.queriedGlobalProcs.count("vkEnumerateInstanceExtensionProperties"));
    ASSERT_EQ(1, getProc.queriedGlobalProcs.count("vkEnumerateInstanceLayerProperties"));
}

TEST_F(LoaderTests, SetInstance) {
    manager.LoadGlobals();
    size_t numGlobals = getProc.queriedGlobalProcs.size();

    manager.SetInstance(getProc.GetInstance());

    // SetInstance shouldn't load any new global functions
    ASSERT_EQ(numGlobals, getProc.queriedGlobalProcs.size());

    // SetInstance should have loaded a bunch of functions (core Vulkan 1.0 has 137 instance functions)
    ASSERT_LT(100, getProc.queriedInstanceProcs.size());

    // SetInstance should have loaded core functions
    ASSERT_EQ(1, getProc.queriedInstanceProcs.count("vkCmdPushConstants"));

    // SetInstance should have loaded extension functions
    ASSERT_EQ(1, getProc.queriedInstanceProcs.count("vkDebugReportMessageEXT"));
}

// Variables used to communicate between MyBeginCommandBuffer and the test
bool BCBCalled = false;
vk::CommandBuffer BCBBuffer;
vk::CommandBufferBeginInfo BCBInfo;

vk::Result MyBeginCommandBuffer(vk::CommandBuffer commandBuffer, const vk::CommandBufferBeginInfo* pBeginInfo) {
    EXPECT_EQ(BCBBuffer, commandBuffer);
    EXPECT_EQ(&BCBInfo, pBeginInfo);
    BCBCalled = true;

    return vk::Result::Incomplete;
}

// Variables used to communicate between MyDestroyDebugReport and the test
bool DDRCalled = false;
vk::Instance DDRInstance;
vk::DebugReportCallbackEXT DDRCallback;

void MyDestroyDebugReport(vk::Instance instance, vk::DebugReportCallbackEXT callback, const vk::AllocationCallbacks* allocator) {
    EXPECT_EQ(DDRInstance, instance);
    EXPECT_EQ(0, memcmp(&DDRCallback, &callback, sizeof(callback)));
    EXPECT_EQ(nullptr, allocator);
    DDRCalled = true;
}

TEST_F(LoaderTests, CallingFunctions) {
    getProc.AddDefault("vkBeginCommandBuffer", reinterpret_cast<vk::UntypedFnptr>(MyBeginCommandBuffer));
    getProc.AddDefault("vkDestroyDebugReportCallbackEXT", reinterpret_cast<vk::UntypedFnptr>(MyDestroyDebugReport));

    manager.LoadGlobals();
    manager.SetInstance(getProc.GetInstance());

    // Check that the arguments are forwarded correctly to the right functions - for core functions
    // Also tests the return value.
    BCBBuffer = reinterpret_cast<vk::CommandBufferImpl*>(this);
    ASSERT_FALSE(BCBCalled);
    vk::Result result = vulkan.BeginCommandBuffer(BCBBuffer, &BCBInfo);
    ASSERT_TRUE(BCBCalled);
    ASSERT_EQ(vk::Result::Incomplete, result);

    // Check that the arguments are forwarded correctly to the right functions - for extensions
    DDRInstance = getProc.GetInstance();
    uint64_t callback = 42;
    DDRCallback = *reinterpret_cast<vk::DebugReportCallbackEXT*>(&callback);

    ASSERT_FALSE(DDRCalled);
    debugReport.DestroyDebugReportCallbackEXT(DDRInstance, DDRCallback, nullptr);
    ASSERT_TRUE(DDRCalled);
}
