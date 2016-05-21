//* PrototypeRenderer Source Code
//* Copyright (c) 2014-2016, Daemon Developers
//* All rights reserved.
//*
//* Redistribution and use in source and binary forms, with or without
//* modification, are permitted provided that the following conditions are met:
//*
//* * Redistributions of source code must retain the above copyright notice, this
//*   list of conditions and the following disclaimer.
//*
//* * Redistributions in binary form must reproduce the above copyright notice,
//*   this list of conditions and the following disclaimer in the documentation
//*   and/or other materials provided with the distribution.
//*
//* * Neither the name of Daemon CBSE nor the names of its
//*   contributors may be used to endorse or promote products derived from
//*   this software without specific prior written permission.
//*
//* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// THIS FILE IS AUTO-GENERATED, EDIT AT YOUR OWN RISK
{% import 'TemplateUtils.h' as utils %}

#include "{{extension.filename}}.h"

#include "vulkan/vulkan.h"
#include "vkcpp/LoaderManager.h"

namespace vk {
    {% set ClassName = extension.name.CamelCase() + 'Loader' %}

    template<typename To, typename From>
    To force_cast(const From& from) {
        return *reinterpret_cast<const To*>(&from);
    }

    {% set globalFunctionNames = [
        'EnumerateInstanceExtensionProperties',
        'EnumerateInstanceLayerProperties',
        'CreateInstance',
    ] %}

    void {{ClassName}}::LoadGlobalFunctions() {
        {% for function in functions %}
            {% if function.name.CamelCase() in globalFunctionNames %}
                {{function.name.camelCase()}}_ = manager.GetGlobalFunction("vk{{function.name.CamelCase()}}");
            {% endif %}
        {% endfor %}
    }
    void {{ClassName}}::LoadInstanceFunctions() {
        {% for function in functions %}
            {% if not function.name.CamelCase() in globalFunctionNames %}
                {{function.name.camelCase()}}_ = manager.GetInstanceFunction("vk{{function.name.CamelCase()}}");
            {% endif %}
        {% endfor %}
    }

    {% for function in functions %}
        {{function.return_type.name.Typename()}} {{ClassName}}::{{function.name.CamelCase()}}(
            {%- call(param) utils.comma_foreach(function.params) -%}
                {{utils.annotated_type(param)}} {{utils.annotated_name(param)}}
            {%- endcall -%}
        ) const {
            {% set returns_void = function.return_type.name.Typename() != 'void' %}
            auto cFnPtr = reinterpret_cast<PFN_vk{{function.name.CamelCase()}}>({{function.name.camelCase()}}_);
            {% if returns_void %}
                auto result ={{' '}}
            {%- endif %}
            cFnPtr(
                {%- call(param) utils.comma_foreach(function.params) -%}
                    force_cast<{{utils.annotated_type(param, native=True, array_to_pointer=True)}}>({{param.name.camelCase()}})
                {%- endcall -%}
            );
            {% if returns_void %}
                return force_cast<{{function.return_type.name.Typename()}}>(result);
            {% endif %}
        }
    {% endfor %}
}
