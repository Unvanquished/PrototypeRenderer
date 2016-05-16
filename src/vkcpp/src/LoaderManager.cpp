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

#include "vkcpp/LoaderManager.h"

#include "vulkan/vulkan.h"

namespace vk {

    LoaderManager::LoaderManager(UntypedFnptr getInstanceProcAddr)
    : untypedGetProc(getInstanceProcAddr) {
    }

    UntypedFnptr LoaderManager::GetGlobalFunction(const char* name) const {
        auto getProc = reinterpret_cast<PFN_vkGetInstanceProcAddr>(untypedGetProc);
        return reinterpret_cast<UntypedFnptr>(getProc(nullptr, name));
    }

    UntypedFnptr LoaderManager::GetInstanceFunction(const char* name) const {
        auto getProc = reinterpret_cast<PFN_vkGetInstanceProcAddr>(untypedGetProc);
        return reinterpret_cast<UntypedFnptr>(getProc(reinterpret_cast<VkInstance>(instance), name));
    }

    void LoaderManager::RegisterLoader(FunctionLoader* loader) {
        loaders.push_back(loader);
    }

    void LoaderManager::LoadGlobals() {
        for (auto loader : loaders) {
            loader->LoadGlobalFunctions();
        }
    }

    void LoaderManager::SetInstance(vk::Instance instance) {
        this->instance = instance;
        for (auto loader : loaders) {
            loader->LoadInstanceFunctions();
        }
    }

}
