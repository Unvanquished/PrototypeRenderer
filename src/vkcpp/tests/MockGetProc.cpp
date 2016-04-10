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

#include "MockGetProc.h"

#include "gtest/gtest.h"

// A C function pointer is passed to the loader manager for vkGetProcAddress.
// During the tests the mock  will be registered in a global variable so that
// GetGlobalProc and GetInstanceProc can access it.
MockGetProc* currentGetProc = nullptr;

void DefaultFunction() {}

vk::UntypedFnptr GetProcProxy(vk::Instance instance, const char* name) {
    EXPECT_NE(nullptr, currentGetProc);
    return currentGetProc->GetProc(instance, name);
}

MockGetProc::MockGetProc() {
    EXPECT_EQ(nullptr, currentGetProc);
    currentGetProc = this;
    AddDefault("vkGetInstanceProcAddr", GetVkGetProcAddress());
}

MockGetProc::~MockGetProc() {
    EXPECT_NE(nullptr, currentGetProc);
    instance = reinterpret_cast<vk::InstanceImpl*>(this);
    currentGetProc = nullptr;
}

vk::UntypedFnptr MockGetProc::GetVkGetProcAddress() const {
    return reinterpret_cast<vk::UntypedFnptr>(GetProcProxy);
}

vk::Instance MockGetProc::GetInstance() const {
    return instance;
}

void MockGetProc::AddDefault(const char* name, vk::UntypedFnptr proc) {
    defaultProcs.insert({name, proc});
}

vk::UntypedFnptr MockGetProc::GetProc(vk::Instance instance, const char* name) {
    EXPECT_EQ(0, queriedGlobalProcs.count(name));
    EXPECT_EQ(0, queriedInstanceProcs.count(name));

    if (instance == nullptr) {
        queriedGlobalProcs.insert(name);
    } else {
        EXPECT_EQ(instance, this->instance);
        queriedInstanceProcs.insert(name);
    }

    auto it = defaultProcs.find(name);
    if (it != defaultProcs.end()) {
        return it->second;
    }
    return DefaultFunction;
}
