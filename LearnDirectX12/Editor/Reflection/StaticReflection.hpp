/*************************************************************************
    > File Name: MetaMacro.hpp
    > Author: Netcan
    > Descripton: MetaMacro
    > Blog: https://netcan.github.io/
    > Mail: 1469709759@qq.com
    > Created Time: 2020-08-30 08:56
************************************************************************/
#pragma once
#include <string_view>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>
#include "Reflection/Reflection.hpp"

namespace refl {

template<char... C>
struct Symbol {
    constexpr static bool symbolTag = true;
    constexpr static size_t length = sizeof...(C);
    constexpr static char string[] = { C..., '\0' };
    constexpr static std::string_view value{ string };
};

template<typename T, size_t... N>
constexpr decltype(auto) _SymbolPrepareImpl(T, std::index_sequence<N...>) {
    return Symbol<T::get()[N]...>{};
}

template<typename T>
constexpr decltype(auto) _SymbolPrepare(T t) {
    using IndexSequence = std::make_index_sequence<sizeof(T::get()) - 1>;
    return _SymbolPrepareImpl(t, IndexSequence{});
}

#define MAKE_SYMBOL(s)                                              \
decltype(::refl::_SymbolPrepare(                                  \
	[]() {                                                      \
	    struct Tmp {                                            \
            constexpr static decltype(auto) get() noexcept {    \
                return s;                                       \
            }                                                   \
	    };                                                      \
        return Tmp{};                                           \
    } () )                                                      \
)

// 静态反射, 没有运行时开销
template<typename T>
struct StaticRefl {
    static_assert(FieldCountV<T> > 0, "Reflection is not supported for this type");
    using RawType = std::decay_t<T>;

    template<size_t I>
    constexpr static decltype(auto) getValue(T &obj) noexcept {
        static_assert(I < FieldCountV<RawType>, "Index out of range");
        return Field<RawType, I>::getValue(obj);
    }

    template<size_t I>
    constexpr static decltype(auto) getValue(const T &obj) noexcept {
        static_assert(I < FieldCountV<RawType>, "Index out of range");
        return Field<RawType, I>::getValue(obj);
    }

    template<size_t I>
    consteval static std::string_view getName() noexcept {
        static_assert(I < FieldCountV<RawType>, "Index out of range");
        return Field<RawType, I>::getName();
    }

    template<typename Symbol, int I = 0>
    consteval static int getIndexByName() noexcept {
        static_assert(Symbol::symbolTag, "Create strings using MAKE_SYMBOL");
        if constexpr (Symbol::value == Field<RawType, I>::getName()) {
            return I;
        } else {
            if constexpr ((I+1) >= FieldCountV<RawType>)
				return -1;
            else
                return getIndexByName<Symbol, I+1>();
        }
    }

    template<typename Symbol, int I = 0>
    constexpr static decltype(auto) getValueByName(T &obj) {
        static_assert(Symbol::symbolTag, "Create strings using MAKE_SYMBOL");
        if constexpr (Symbol::value == Field<T, I>::getName()) {
            return Field<RawType, I>::getValue(obj);
        } else {
            static_assert(I < FieldCountV<RawType>, "There are no matching reflection variables");
            return getValueByName<Symbol, I+1>(obj);
        }
    }

    template<typename Symbol, int I = 0>
    constexpr static decltype(auto) getValueByName(const T &obj) {
        static_assert(Symbol::symbolTag, "Create strings using MAKE_SYMBOL");
        if constexpr (Symbol::value == Field<RawType, I>::getName()) {
			return Field<RawType, I>::getValue(obj);
        } else {
            static_assert(I < FieldCountV<RawType>, "There are no matching reflection variables");
            return getValueByName<Symbol, I + 1>(obj);
        }
    }

    template<size_t I = 0, typename F>
    constexpr static void foreach(T &obj, F &&func) {
        if constexpr (I < FieldCountV<RawType>) {
            func(Field<RawType, I>::getName(), Field<RawType, I>::getValue(obj));
	        return foreach<I+1>(obj, func);
        }
    }

    template<size_t I = 0, typename F>
    constexpr static void foreach(const T &obj, F &&func) {
        if constexpr (I < FieldCountV<RawType>) {
            func(Field<RawType, I>::getValue(obj));
            return foreach<I+1>(obj, func);
        }
    }
};

}