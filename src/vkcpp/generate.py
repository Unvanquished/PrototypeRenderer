#!/usr/bin/python

# PrototypeRenderer Source Code
# Copyright (c) 2014-2016, Daemon Developers
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
# * Neither the name of Daemon CBSE nor the names of its
#   contributors may be used to endorse or promote products derived from
#   this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import xml.etree.ElementTree
import jinja2
import re
import argparse
import os
from collections import namedtuple, OrderedDict

#TODO(kangz) do not lower the extensions vendor name and somehow keep ASTC_4x4 instead of ASTC_4X4

# After parsing all names are stored as a list of lowerase words so that conversion to
# other name formatting is possible. For example in vulkan.h enum values are in SNAKE_CASE
# but in the generated code we want them to be in CamelCase.

def split_camelCase(name):
    assert(len(name) > 0 and name[0].islower())
    split = []
    start = 0
    for (i, char) in enumerate(name):
        if char.isupper() and (i == 0 or not name[i - 1].isupper()):
            split.append(name[start:i].lower())
            start = i
    split.append(name[start:].lower())
    return split

def split_CamelCase(name):
    assert(len(name) > 0 and name[0].isupper())
    name = name[0].lower() + name[1:]
    return split_camelCase(name)

def split_SNAKE_CASE(name):
    # Can't assert on this because of ASTC_4x4
    # assert(name.isupper())
    return [chunk.lower() for chunk in name.split('_')]

def split_Typename(name):
    """Splits Vulkan typenames but not system typenames"""
    if name.startswith('Vk') or name.startswith('PFN_vk'):
        return split_CamelCase(name)
    return [name]

# In vulkan enums us regular C enums so the values are prefixed with a SNAKE_CASE version
# of the enum type name. With scoped enums we don't want this so we factor out the type name
# from the value name.
def factor_name(name, factorand):
    assert(len(name) > len(factorand))
    # When factoring we need to take care of the vendor suffixes that can be present, which means
    # we need to factor both in the start and the end of the name.
    first_difference = 0
    for i in range(len(factorand)):
        if name[i] != factorand[i]:
            first_difference = i
            break
        first_difference = i + 1

    rest = factorand[first_difference:]
    if len(rest) == 0:
        return name[first_difference:]
    else:
        #TODO check that rest is a vendor suffix.
        assert(len(rest) == 1) # Vendor suffixes are always one word
        assert(rest == name[-len(rest):])
        return name[first_difference:-len(rest)]

class Name:
    def __init__(self, chunks):
        self.chunks = chunks

    def strip_vk(self, chunks):
        if chunks[0] in ('PFN_vk', 'vk'):
            return list(chunks[1:])
        return chunks

    def CamelChunk(self, chunk):
        if chunk in ['1d', '2d', '3d']:
            return chunk.upper()
        return chunk[0].upper() + chunk[1:]

    def canonical_case(self):
        return '_'.join(self.chunks)

    def camelCase(self):
        chunks = self.strip_vk(self.chunks)
        return ''.join(chunks[0] + map(lambda chunk: self.CamelChunk(chunk), chunks[1:]))

    def CamelCase(self):
        chunks = self.strip_vk(self.chunks)
        return ''.join(map(lambda chunk: self.CamelChunk(chunk), chunks))

    def EnumCase(self):
        result = self.CamelCase()
        if result[0].isdigit():
            return 'e' + result
        return result

# For structure definitions or function parameters we need to store both a type and a name
# but we also need to get any modifiers to the type such as *, const*, being an array, etc.
class AnnotatedTypeAndName:
    def __init__(self):
        self.name = None
        self.typ = []

        self.annotation = ""
        self.integral_count = 0
        self.constant_count = []

    def parse_regular_parameter(self, element):
        # Parameters or structure members are like the following:
        #     <param>STUFF<type>bar</type>STUFF<name>baz</name>STUFF</param>
        # With stuff containing C style type modifiers, and <param> being <memeber> for structures.
        name_node = element.find('name')
        type_node = element.find('type')

        self.name = Name(split_camelCase(name_node.text))
        self.typ = Name(split_Typename(type_node.text))

        # To parse annotations we gather the text that is between child elements and match
        # it against a list of known patterns. Note that arrays sized with Vulkan constant
        # look like the following:
        #     <param><type>bar</type><name>foo</name>[<enum>CONSTANT_NAME</enum>]</param>
        def sanitize_annotation_chunk(chunk):
            if chunk == None:
                return ''
            return chunk.replace(' ', '')

        annotation = [sanitize_annotation_chunk(element.text)]
        for child in element:
            annotation.append(sanitize_annotation_chunk(child.tail))

        def make_constant_array():
            self.annotation = '[]'
            self.constant_count = split_SNAKE_CASE(element.find('enum').text)

        def make_integral_array(count, const):
            self.annotation = 'const[]' if const else '[]'
            self.integral_count = count

        possible_annotations = [
            (['', '', ''], ''),
            (['', '', '[', ']'], make_constant_array),
            (['', '*', ''], '*'),
            (['', '**', ''], '**'),
            (['', '', '[2]'], lambda: make_integral_array(2, False)),
            (['', '', '[3]'], lambda: make_integral_array(3, False)),
            (['', '', '[4]'], lambda: make_integral_array(4, False)),
            (['const', '*', ''], 'const*'),
            (['const', '', '[4]'], lambda: make_integral_array(4, True)),
            (['const', '*const*', ''], 'const*const*'),
            (['struct', '*', ''], 'struct*'),
        ]

        found = False
        for (annotation_type, annotate) in possible_annotations:
            if annotation == annotation_type:
                found = True
                if isinstance(annotate, str):
                    self.annotation = annotate
                else:
                    annotate()
        assert(found)

    def parse_funcpointer(self, element, index):
        # Function pointer parameter definitions are very loosely structured, all we get
        # is the name of the type in a <type> element, all the rest we have to guess.
        assert(index >= 2 and element[index].tag == 'type')
        node = element[index]
        self.typ = Name(split_Typename(node.text))

        # Currently there are only 3 types of annotations for function pointer parameters:
        # "", "*", "const*" so we can manually check which one it is. 
        split_tail = list(filter(lambda chunk: len(chunk) > 0, node.tail.split(' ')))

        if split_tail[0] == '*':
            name_part = split_tail[1]

            previous_node = element[index - 1]
            previous_split = list(filter(lambda chunk: len(chunk) > 0, previous_node.tail.split(' ')))
            if previous_split[-1] == 'const':
                self.annotation = 'const*'
            else:
                self.annotation = '*'
        else:
            name_part = split_tail[0]

        self.name = Name(split_camelCase(''.join(filter(lambda char: char.isalnum(), name_part))))

    def link(self, types):
        self.typ = types[self.typ.canonical_case()]

class Constant:
    def __init__(self, element):
        # Constants are defined as follows:
        #     <enum value="42" name="ANSWER"/>
        # TODO(kangz) values can be float, int, unsigned longs, etc.
        # if we want to make them constexpr we'll have to guess the type
        assert(element.tag == 'enum')
        self.name = Name(split_SNAKE_CASE(element.attrib['name']))
        self.value = element.attrib['value']

EnumValue = namedtuple('EnumValue', ['name', 'value'])
class EnumType:
    def __init__(self, element, factor):
        # Enums are defined as follows:
        #     <enums name="VkFoo" type="enum">
        #         <enum value="42" name="VK_FOO_BAR"/> ...
        #     </enums>
        self.name = Name(split_CamelCase(element.attrib['name']))
        self.values = []

        for child in element:
            # VkResult has got an <unused/> child, skip it
            if child.tag != 'enum':
                assert(child.tag == 'unused')
                continue

            name = split_SNAKE_CASE(child.attrib['name'])
            if factor:
                name = Name(factor_name(name, self.name.chunks))
            else:
                name = Name(name)
            self.values.append(EnumValue(name, int(child.attrib['value'], 0)))

        self.values.sort(key=lambda value: value.value)

    def link(self, types):
        pass

BitmaskBit = namedtuple('BitmaskBit', ['name', 'bit'])
BitmaskValue = namedtuple('BitmaskValue', ['name', 'value'])
class BitmaskType:
    def __init__(self, element):
        self.bits = []
        self.values = []

        if element.tag == 'type':
            self.name = Name(split_CamelCase(element.find('name').text))
            return

        # Bitmasks are defined as follows:
        #     <enums name="VkFoo" type="bitmask">
        #         <enum bitpos="42" name="VK_FOO_BAR"/> ...
        #     </enums>
        self.name = split_CamelCase(element.attrib['name'])

        # Bitmask names always finish with FlagBits with an optional vendor suffix
        has_extension = False
        if len(self.name) >= 3 and self.name[-2:] == ['flag', 'bits']:
            self.name = self.name[:-2]
        elif len(self.name) >= 4 and self.name[-3:-1] == ['flag', 'bits']:
            has_extension = True
            self.name = self.name[:-3] + self.name[-1:]
        self.name = Name(self.name)

        for child in element:
            name = split_SNAKE_CASE(child.attrib['name'])
            name = Name(factor_name(name, self.name.chunks))

            # Some bitmask use values which are not exact bits in order to have preset
            # combinations, such as cull mode front and back (which is 0b01 | 0b10 == 0b11)
            if 'value' in child.attrib:
                self.values.append(BitmaskValue(name, int(child.attrib['value'], 0)))
            else:
                assert('bitpos' in child.attrib)
                self.bits.append(BitmaskBit(name, int(child.attrib['bitpos'], 0)))

        self.values.sort(key=lambda value: value.value)

        # In vk.xml bitmaks are defined in two parts:
        #     <type requires="VkFooFlagBits" category="bitmask">typdef<type>VkFlags</type><name>VkFooFlags</name></type>
        #     The Bitmask definition as VkFooFlagBits
        # Others parts of the definition refer to this type as VkFooFlags so we need to add "Flags" to the name
        if has_extension:
            self.name.chunks = self.name.chunks[-1:] + ['flags'] + self.name.chunks[:-1]
        else:
            self.name.chunks.append('flags')

    def link(self, types):
        pass

class SystemType:
    def __init__(self, element):
        # System types are defined as follows:
        #     <type requires="header.h" name="foo"/>
        assert('name' in element.attrib and 'requires' in element.attrib)
        self.name = Name([element.attrib['name']])
        self.header = element.attrib['requires']

    def link(self, types):
        pass

class BaseType:
    def __init__(self, element):
        # Base types are defined as follows:
        #     <type category="basetype">typedef <type>system</type> <name>base</name>;</type>
        self.name = Name(split_CamelCase(element.find('name').text))
        self.base_type = element.find('type').text

    def link(self, types):
        pass

class HandleType:
    def __init__(self, element):
        # Handle types are defined as follows:
        #     <type category="handle" parent="foo"><type>VK_DEFINE_HANDLE</type>(<name>name</name>)</type>
        # Where VK_DEFINE_HANDLE can also be VK_DEFINE_NON_DISPATCHABLE_HANDLE
        self.name = Name(split_CamelCase(element.find('name').text))
        self.dispatchable = element.find('type').text == 'VK_DEFINE_HANDLE'

    def link(self, types):
        pass


class StructMember(AnnotatedTypeAndName):
    def __init__(self, element):
        AnnotatedTypeAndName.__init__(self)
        self.parse_regular_parameter(element)
        self.optional = 'optional' in element.attrib and element.attrib['optional'] == 'true'

class StructType:
    def __init__(self, element, is_union):
        # Structure types are defined as follows:
        #   <type category="struct" name="foo">
        #       <member><type>bar</type><name>baz</name></member>
        #   </type>
        # The member tag can in addition have decoration to make it a pointer, an array, etc.
        self.name = Name(split_CamelCase(element.attrib['name']))
        self.is_union = is_union
        self.members = []

        for child in element:
            if child.tag == 'member':
                self.members.append(StructMember(child))
            else:
                assert(child.tag == 'validity')

    def link(self, types):
        for member in self.members:
            member.link(types)

class FunctionParam(AnnotatedTypeAndName):
    def __init__(self, element=None):
        AnnotatedTypeAndName.__init__(self)

        if element != None:
            self.optional = 'optional' in element.attrib and element.attrib['optional'] == 'true'
        else:
            self.optional = False

class FnptrType:
    def __init__(self, element):
        # Function pointer types are defined as follows:
        #     <type category="funcpointer">typedef <type>foo</type> JUNK <name>PFN_vkBar</name>
        #         <type>baz</type> bazNameJUNK ...
        #     </type>
        # Note that some annotations can be put before or after the params <type> tag.
        assert(element[0].tag == 'type' and element[1].tag == 'name')
        self.name = Name(split_CamelCase(element.find('name').text))
        self.return_type = Name(split_Typename(element.find('type').text))

        self.params = []
        for i in range(2, len(element)):
            param = FunctionParam()
            param.parse_funcpointer(element, i)
            self.params.append(param)

    def link(self, types):
        self.return_type = types[self.return_type.canonical_case()]
        for param in self.params:
            param.link(types)


class Function:
    def __init__(self, element):
        # Functions are defined as follows:
        # <command>
        #     <proto><type>toto</type> <name>foo</name></proto>
        #     <param><type>bar</type><name>baz</name></param>
        # </command>
        # The parameter can in addition have decoration to make it a pointer, an array, etc.
        proto = element.find('proto')

        # Make sure the return value doesn't have annotations
        assert(proto.text == None and proto[0].tail == ' ' and proto[1].tail == None)
        assert(not '*' in proto.find('type').text)

        self.name = Name(split_camelCase(proto.find('name').text))
        self.return_type = Name(split_Typename(proto.find('type').text))

        self.params = []
        for child in element:
            if child.tag == 'param':
                param = FunctionParam(child)
                param.parse_regular_parameter(child)
                self.params.append(param)
            else:
                assert(child.tag in ('proto', 'validity', 'implicitexternsyncparams'))

    def link(self, types):
        self.return_type = types[self.return_type.canonical_name()]
        for param in self.params:
            param.link(types)

ExtensionEnumValue = namedtuple('ExtensionEnumValue', ['name', 'extends', 'value'])
class Extension:
    def __init__(self, element):
        # Extension (or "features") are defined as follows:
        #     <extension name="Foo" ...>
        #         <require>
        #             <type name="bar"/>
        #             <command name="baz"/>
        #             <enum offset='42' extends='foo' name='bar'/>
        #         </require>
        #     </extension>
        extension_number = 0
        if not 'api' in element.attrib:
            extension_number = int(element.attrib['number'], 0)

        self.name = Name(split_SNAKE_CASE(element.attrib['name']))

        self.required_types = []
        self.required_commands = []
        self.enum_values = []

        for require in element:
            assert(require.tag == 'require')

            for required in require:
                if required.tag == 'type':
                    self.required_types.append(Name(split_Typename(required.attrib['name'])))

                elif required.tag == 'command':
                    self.required_commands.append(Name(split_camelCase(required.attrib['name'])))

                elif required.tag == 'enum':
                    # Some enum tags define the version of the extension, skip them.
                    if not 'extends' in required.attrib:
                        continue

                    extends = Name(split_Typename(required.attrib['extends']))
                    name = Name(split_SNAKE_CASE(required.attrib['name']))

                    direction = +1
                    if 'dir' in required.attrib and required.attrib['dir'] == '-':
                        direction = -1

                    # Compute the extension enum value as in defined in the specification.
                    offset = int(required.attrib['offset'], 0)
                    value = direction * (1000000000 + (extension_number - 1) * 1000 + offset)

                    self.enum_values.append(ExtensionEnumValue(name, extends, value))

# A custom Jinja2 template loader that removes the extra indentation
# of the template blocks so that the output is correctly indented
class PreprocessingLoader(jinja2.BaseLoader):
    def __init__(self, path):
        self.path = path

    def get_source(self, environment, template):
        path = os.path.join(self.path, template)
        if not os.path.exists(path):
            raise jinja2.TemplateNotFound(template)
        mtime = os.path.getmtime(path)
        with open(path) as f:
            source = self.preprocess(f.read())
        return source, path, lambda: mtime == os.path.getmtime(path)

    def preprocess(self, source):
        # Remove the trailing newline
        lines = source.split("\n")

        # Compute the current indentation level of the template blocks and remove their indentation
        result = []
        indentation_level = 0
        for line in lines:
            if self.blockstart.match(line) != None:
                indentation_level += 1
            elif self.blockend.match(line) != None:
                indentation_level -=1

            result.append(self.remove_indentation(line, indentation_level))

        return self.my_filter('\n'.join(result))

    def remove_indentation(self, line, n):
        for _ in range(n):
            if line.startswith(' '):
                line = line[4:]
            elif line.startswith('\t'):
                line = line[1:]
        return line

    blockstart = re.compile('^\s*{%-?\s*(if|for).*$')
    blockend = re.compile('^\s*{%-?\s*end(if|for).*$')

    def my_filter(self, text):
        lines = []
        for line in text.split('\n'):
            # Remove the "template" comments
            if line.strip().startswith("//*"):
                continue

            lines.append(line)
        return '\n'.join(lines)

def parse_vulkan_xml(filename):
    constants = []
    enum_types = []
    bitmask_types = []
    system_types = []
    base_types = []
    handle_types = []
    struct_types = []
    fnptr_types = []

    functions = []

    main_api = None
    extensions = []

    root = xml.etree.ElementTree.parse(filename).getroot()

    for enum in root.iter('enums'):
        # Some random constants are defined inside an enum, skip them.
        if enum.attrib['name'] == 'API Constants':
            for child in enum:
                constants.append(Constant(child))

        elif enum.attrib['type'] == 'enum':
            # VkResult values are not namespaced in C Vulkan so we can't factor the enum name out
            factor = enum.attrib['name'] != 'VkResult'
            enum_types.append(EnumType(enum, factor))

        elif enum.attrib['type'] == 'bitmask':
            bitmask_types.append(BitmaskType(enum))

    types_node = root.find('types')
    for typ in types_node:
        if typ.tag != 'type':
            pass

        elif not 'category' in typ.attrib:
            system_types.append(SystemType(typ))

        elif typ.attrib['category'] == 'basetype':
            base_types.append(BaseType(typ))

        elif typ.attrib['category'] == 'handle':
            handle_types.append(HandleType(typ))

        elif typ.attrib['category'] == 'struct':
            struct_types.append(StructType(typ, False))

        elif typ.attrib['category'] == 'union':
            struct_types.append(StructType(typ, True))

        elif typ.attrib['category'] == 'funcpointer':
            fnptr_types.append(FnptrType(typ))

        # Some empty bitmasks have a typedef but no enum definition
        elif typ.attrib['category'] == 'bitmask' and not 'require' in typ.attrib:
            bitmask_types.append(BitmaskType(typ))

    for element in root.find('commands'):
        assert(element.tag == 'command')
        functions.append(Function(element))

    assert(len(root.findall('feature')) == 1)
    main_api = Extension(root.find('feature'))

    for extension in root.find('extensions'):
        assert(extension.tag == 'extension')
        extensions.append(Extension(extension))

    types = {}
    for typ in enum_types + bitmask_types + system_types + base_types + handle_types + struct_types + fnptr_types:
        types[typ.name.canonical_case()] = typ

    for typ in types.values():
        typ.link(types)

    # TODO(kangz) link the extensions and functions

    return {
        'constants': constants,
        'enum_types': enum_types,
        'bitmask_types': bitmask_types,
        'system_types': system_types,
        'base_types': base_types,
        'handle_types': handle_types,
        'struct_types': struct_types,
        'fnptr_types': fnptr_types,
        'functions': functions,
        'main_api': main_api,
        'extensions': extensions,
        'types': types,
    }

#TODO(kangz)
# - Analyze
#   - Link types
#   - Order types (alphabetical sort + stable topo sort)
#   - Gather requirements per extension
# - Output
#   - C++ type definitions
#   - Functions loaders and C++ wrappers for
#     - Global functions
#     - Instance functions
#     - Device functions
#     - static_assert file for types
# - Stretch
#   - Overload count and pointer return values by std::vector
#   - Do not require any patching of vk.xml

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description = 'Outputs a C++ wrapper for the Vulkan C API.',
        formatter_class = argparse.ArgumentDefaultsHelpFormatter
    )
    parser.add_argument('xml', metavar='VULKAN_XML', nargs=1, type=open, help ='The Vulkan XML definition to use.')
    parser.add_argument('-t', '--template-dir', default="templates", type=str, help='Directory with template files.')
    parser.add_argument('-o', '--output-dir', default=None, type=str, help='Output directory for the generated source files.')

    args = parser.parse_args()

    vulkan = parse_vulkan_xml(args.xml[0])
    args.xml[0].close()

    env = jinja2.Environment(loader=PreprocessingLoader(args.template_dir), trim_blocks=True, lstrip_blocks=True)

    if args.output_dir != None:
        # Generate a list of files to create, params_dicts will get squashed to create the template parameters
        FileToRender = namedtuple('FileToRender', ['template', 'output', 'params_dicts'])
        to_render = []

        base_dir = args.output_dir + os.path.sep

        to_render.append(FileToRender('Vulkan.h', base_dir + 'Vulkan.h', [vulkan]))

        for render in to_render:
            params = OrderedDict()
            for param_dict in render.params_dicts:
                params.update(param_dict)
            content = env.get_template(render.template).render(**params) + "\n"

            directory = os.path.dirname(render.output)
            if not os.path.exists(directory):
                os.makedirs(directory)

            with open(render.output, 'w') as outfile:
                outfile.write(content)
