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
{% import 'TemplateUtils.h' as utils %}

#include "{{extension.filename}}.h"
#include "vulkan/vulkan.h"

#include <type_traits>
#include <cstddef>

using namespace vk;

template<typename A, typename B>
struct Compatible {
    static constexpr bool value = sizeof(A) == sizeof(B) && alignof(A) == alignof(B);
};

{% for type in base_types %}
    static_assert(std::is_same<{{type.name.Typename()}}, {{type.base_type.name.Typename()}}>::value, "");
{% endfor %}

{% for typ in enum_types %}
    static_assert(Compatible<{{typ.name.Typename()}}, ::{{typ.name.nativeTypename()}}>::value, "");
{% endfor %}

{% for typ in bitmask_types %}
    static_assert(Compatible<{{typ.name.Typename()}}, ::{{typ.name.nativeTypename()}}>::value, "");
{% endfor %}

{% for typ in handle_types %}
    static_assert(Compatible<{{typ.name.Typename()}}, ::{{typ.name.nativeTypename()}}>::value, "");
{% endfor %}

{% for typ in fnptr_types %}
    static_assert(Compatible<{{typ.name.Typename()}}, ::{{typ.name.nativeTypename()}}>::value, "");
{% endfor %}

{% for typ in struct_types %}
    static_assert(Compatible<{{typ.name.Typename()}}, ::{{typ.name.nativeTypename()}}>::value, "");
    {% for member in typ.members %}
        static_assert(offsetof({{typ.name.Typename()}}, {{member.name.camelCase()}}) == offsetof(::Vk{{typ.name.Typename()}}, {{member.name.camelCase()}}), "");
    {% endfor %}

{% endfor %}
