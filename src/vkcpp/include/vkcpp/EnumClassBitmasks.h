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

#ifndef VKCPP_ENUM_CLASS_BITMASKS_H_
#define VKCPP_ENUM_CLASS_BITMASKS_H_

#include <type_traits>

namespace vk {

    template<typename T>
    struct IsVkBitmask {
        static constexpr bool enable = false;
    };

    template<typename T, typename Enable = void>
    struct LowerBitmask {
        static constexpr bool enable = false;
    };

    template<typename T>
    struct LowerBitmask<T, typename std::enable_if<IsVkBitmask<T>::enable>::type> {
        static constexpr bool enable = true;
        using type = T;
        static T Lower(T t) {return t;}
    };

    template<typename T>
    struct BoolConvertible {
        using base_type = T;
        using integral = typename std::underlying_type<T>::type;

        BoolConvertible(integral value) : value(value) {}
        operator bool() const {return value != 0;}
        operator T() const {return static_cast<T>(value);}

        integral value;
    };

    template<typename T>
    struct LowerBitmask<BoolConvertible<T>> {
        static constexpr bool enable = true;
        using type = T;
        static type Lower(BoolConvertible<T> t) {return t;}
    };

    template<typename T1, typename T2, typename = typename std::enable_if<
        LowerBitmask<T1>::enable && LowerBitmask<T2>::enable
    >::type>
    constexpr BoolConvertible<typename LowerBitmask<T1>::type>
    operator | (T1 left, T2 right) {
        using T = typename LowerBitmask<T1>::type;
        using integral = typename std::underlying_type<T>::type;
        T l = LowerBitmask<T1>::Lower(left);
        T r = LowerBitmask<T2>::Lower(right);
        return static_cast<integral>(l) | static_cast<integral>(r);
    }
    template<typename T1, typename T2, typename = typename std::enable_if<
        LowerBitmask<T1>::enable && LowerBitmask<T2>::enable
    >::type>
    constexpr BoolConvertible<typename LowerBitmask<T1>::type>
    operator & (T1 left, T2 right) {
        using T = typename LowerBitmask<T1>::type;
        using integral = typename std::underlying_type<T>::type;
        T l = LowerBitmask<T1>::Lower(left);
        T r = LowerBitmask<T2>::Lower(right);
        return static_cast<integral>(l) & static_cast<integral>(r);
    }
    template<typename T1, typename T2, typename = typename std::enable_if<
        LowerBitmask<T1>::enable && LowerBitmask<T2>::enable
    >::type>
    constexpr BoolConvertible<typename LowerBitmask<T1>::type>
    operator ^ (T1 left, T2 right) {
        using T = typename LowerBitmask<T1>::type;
        using integral = typename std::underlying_type<T>::type;
        T l = LowerBitmask<T1>::Lower(left);
        T r = LowerBitmask<T2>::Lower(right);
        return static_cast<integral>(l) ^ static_cast<integral>(r);
    }
    template<typename T1>
    constexpr BoolConvertible<typename LowerBitmask<T1>::type>
    operator ~ (T1 t) {
        using T = typename LowerBitmask<T1>::type;
        using integral = typename std::underlying_type<T>::type;
        return ~static_cast<integral>(LowerBitmask<T1>::Lower(t));
    }

    template<typename T, typename T2, typename = typename std::enable_if<
        IsVkBitmask<T>::enable && LowerBitmask<T2>::enable
    >::type>
    T& operator &= (T& l, T2 right) {
        T r = LowerBitmask<T2>::Lower(right);
        l = l & r;
        return l;
    }
    template<typename T, typename T2, typename = typename std::enable_if<
        IsVkBitmask<T>::enable && LowerBitmask<T2>::enable
    >::type>
    T& operator |= (T& l, T2 right) {
        T r = LowerBitmask<T2>::Lower(right);
        l = l | r;
        return l;
    }
    template<typename T, typename T2, typename = typename std::enable_if<
        IsVkBitmask<T>::enable && LowerBitmask<T2>::enable
    >::type>
    T& operator ^= (T& l, T2 right) {
        T r = LowerBitmask<T2>::Lower(right);
        l = l ^ r;
        return l;
    }
}

#endif // VKCPP_ENUM_CLASS_BITMASKS_H_
