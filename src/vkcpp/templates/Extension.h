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

#ifndef VKCPP_{{extension.name.SNAKE_CASE()}}_H_
#define VKCPP_{{extension.name.SNAKE_CASE()}}_H_

{% for extension in required_extensions %}
    #include "{{extension.filename}}.h"
{% endfor %}

{% block extra_headers %}
{% endblock %}

{% for header in required_headers %}
    {% if header in ['vk_platform.h'] %}
        #include "{{header}}"
    {% else %}
        #include <{{header}}>
    {% endif %}
{% endfor %}

namespace vk {

    {% for type in base_types %}
        using {{type.name.Typename()}} = {{type.base_type.name.Typename()}};
    {% endfor %}

    {% for enum in enum_types %}
        enum class {{enum.name.Typename()}}: int32_t {
            {% for value in enum.values %}
                {{value.name.EnumCase()}} = {{value.value}},
            {% endfor %}
        };

    {% endfor %}

    {% for bitmask in bitmask_types %}
        enum class {{bitmask.name.Typename()}}: int32_t {
            {% for bit in bitmask.bits %}
                {{bit.name.EnumCase()}} = 1 << {{bit.bit}},
            {% endfor %}
            {% for value in bitmask.values %}
                {{value.name.EnumCase()}} = {{value.value}},
            {% endfor %}
        };

    {% endfor %}
    {% for bitmask in bitmask_types %}
        template<>
        struct IsVkBitmask<{{bitmask.name.Typename()}}> {
            static constexpr bool enable = true;
        };

    {% endfor %}

    {% block extra_base_definitions %}
    {% endblock %}

    {% for typ in handle_types %}
        {% if typ.dispatchable %}
            using {{typ.name.Typename()}} = class {{typ.name.Typename()}}Impl*;
        {% else %}
            class {{typ.name.Typename()}} : public NonDispatchableHandle {
                using NonDispatchableHandle::NonDispatchableHandle;
            };
        {% endif %}

    {% endfor %}

    {% for type in fnptr_types %}
        using {{type.name.Typename()}} = {{type.return_type.name.Typename()}} (VKAPI_PTR*) (
            {%- call(param) utils.comma_foreach(type.params) -%}
                {{utils.annotated_type(param)}} {{utils.annotated_name(param)}}
            {%- endcall -%}
        );
    {% endfor %}

    {% for type in struct_types %}
        {% if type.is_union %}
            union
        {%- else %}
            struct
        {%- endif -%}
        {{' '}}{{type.name.Typename()}} {
            {% for member in type.members %}
                {{utils.annotated_type(member)}} {{utils.annotated_name(member)}};
            {% endfor %}
        };

    {% endfor %}

    {% set ClassName = extension.name.CamelCase() + 'Loader' %}
    class {{ClassName}} final : public FunctionLoader {
        public:
            using FunctionLoader::FunctionLoader;

            void LoadGlobalFunctions() override;
            void LoadInstanceFunctions() override;

            {% for function in functions %}
                {{function.return_type.name.Typename()}} {{function.name.camelCase()}}(
                    {%- call(param) utils.comma_foreach(function.params) -%}
                        {{utils.annotated_type(param)}} {{utils.annotated_name(param)}}
                    {%- endcall -%}
                );
            {% endfor %}

        private:
            {% for function in functions %}
                UntypedFnptr {{function.name.camelCase()}}_ = nullptr;
            {% endfor %}
    };
}

#endif // VKCPP_{{extension.name.SNAKE_CASE()}}_H_
