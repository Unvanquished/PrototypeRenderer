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

    {% block extra_base_definitions %}{% endblock %}
    //* TODO(kangz) handle_types
    //* TODO(kangz) struct_types
    //* TODO(kangz) fnptr_types
    //* TODO(kangz) commands
}

#endif // VKCPP_{{extension.name.SNAKE_CASE()}}_H_