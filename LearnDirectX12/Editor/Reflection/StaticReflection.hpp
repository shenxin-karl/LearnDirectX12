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
#include "TypeList.hpp"

#define PP_THIRD_ARG(a, b, c, ...) c
#define VA_OPT_SUPPORTED_I(...) PP_THIRD_ARG(__VA_OPT__(, ), 1, 0, )
#define VA_OPT_SUPPORTED VA_OPT_SUPPORTED_I(?)

// Traditional MSVC requires a special EXPAND phase
#if (defined(_MSC_VER) && !defined(_MSVC_TRADITIONAL)) ||                  \
    (defined(_MSVC_TRADITIONAL) && _MSVC_TRADITIONAL)

#define _GET_ARG_COUNT(...)                                                 \
    INTERNAL_EXPAND_ARGS_PRIVATE(INTERNAL_ARGS_AUGMENTER(__VA_ARGS__))

#define INTERNAL_ARGS_AUGMENTER(...) unused, __VA_ARGS__
#define INTERNAL_EXPAND(x) x
#define INTERNAL_EXPAND_ARGS_PRIVATE(...)                                  \
    INTERNAL_EXPAND(INTERNAL_GET_ARG_COUNT_PRIVATE(                        \
        __VA_ARGS__, 100, 99, 98, 97, 96, 95, 94, 93, 92, 91, 90, 89, 88,  \
        87, 86, 85, 84, 83, 82, 81, 80, 79, 78, 77, 76, 75, 74, 73, 72,    \
        71, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56,    \
        55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40,    \
        39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24,    \
        23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7,   \
        6, 5, 4, 3, 2, 1, 0))

#else  // Other compilers

#if VA_OPT_SUPPORTED  // Standardized in C++20
#define GET_ARG_COUNT(...)                                                 \
    INTERNAL_GET_ARG_COUNT_PRIVATE(                                        \
        unused __VA_OPT__(, ) __VA_ARGS__, 100, 99, 98, 97, 96, 95, 94,    \
        93, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80, 79, 78,    \
        77, 76, 75, 74, 73, 72, 71, 70, 69, 68, 67, 66, 65, 64, 63, 62,    \
        61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46,    \
        45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30,    \
        29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14,    \
        13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#elif defined(__GNUC__)  // Extension in GCC/Clang
#define GET_ARG_COUNT(...)                                                 \
    INTERNAL_GET_ARG_COUNT_PRIVATE(                                        \
        unused, ##__VA_ARGS__, 100, 99, 98, 97, 96, 95, 94, 93, 92, 91,    \
        90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80, 79, 78, 77, 76, 75,    \
        74, 73, 72, 71, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59,    \
        58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43,    \
        42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27,    \
        26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11,    \
        10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#else  // GET_ARG_COUNT() may return 1 here
#define GET_ARG_COUNT(...)                                                 \
    INTERNAL_GET_ARG_COUNT_PRIVATE(                                        \
        unused, __VA_ARGS__, 100, 99, 98, 97, 96, 95, 94, 93, 92, 91, 90,  \
        89, 88, 87, 86, 85, 84, 83, 82, 81, 80, 79, 78, 77, 76, 75, 74,    \
        73, 72, 71, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58,    \
        57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42,    \
        41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26,    \
        25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, \
        8, 7, 6, 5, 4, 3, 2, 1, 0)
#endif

#endif

#define INTERNAL_GET_ARG_COUNT_PRIVATE(                                    \
    e0, e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12, e13, e14, e15,  \
    e16, e17, e18, e19, e20, e21, e22, e23, e24, e25, e26, e27, e28, e29,  \
    e30, e31, e32, e33, e34, e35, e36, e37, e38, e39, e40, e41, e42, e43,  \
    e44, e45, e46, e47, e48, e49, e50, e51, e52, e53, e54, e55, e56, e57,  \
    e58, e59, e60, e61, e62, e63, e64, e65, e66, e67, e68, e69, e70, e71,  \
    e72, e73, e74, e75, e76, e77, e78, e79, e80, e81, e82, e83, e84, e85,  \
    e86, e87, e88, e89, e90, e91, e92, e93, e94, e95, e96, e97, e98, e99,  \
    e100, count, ...)                                                      \
    count

#define _REPEAT_0(func, T, i, arg)
#define _REPEAT_1(func, T, i, arg)        func(T, i, arg)
#define _REPEAT_2(func, T, i, arg, ...)   func(T, i, arg) _PARE(_REPEAT_1(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_3(func, T, i, arg, ...)   func(T, i, arg) _PARE(_REPEAT_2(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_4(func, T, i, arg, ...)   func(T, i, arg) _PARE(_REPEAT_3(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_5(func, T, i, arg, ...)   func(T, i, arg) _PARE(_REPEAT_4(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_6(func, T, i, arg, ...)   func(T, i, arg) _PARE(_REPEAT_5(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_7(func, T, i, arg, ...)   func(T, i, arg) _PARE(_REPEAT_6(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_8(func, T, i, arg, ...)   func(T, i, arg) _PARE(_REPEAT_7(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_9(func, T, i, arg, ...)   func(T, i, arg) _PARE(_REPEAT_8(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_10(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_9(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_11(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_10(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_12(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_11(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_13(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_12(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_14(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_13(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_15(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_14(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_16(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_15(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_17(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_16(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_18(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_17(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_19(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_18(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_20(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_19(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_21(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_20(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_22(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_21(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_23(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_22(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_24(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_23(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_25(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_24(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_26(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_25(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_27(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_26(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_28(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_27(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_29(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_28(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_30(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_29(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_31(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_30(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_32(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_31(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_33(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_32(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_34(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_33(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_35(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_34(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_36(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_35(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_37(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_36(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_38(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_37(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_39(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_38(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_40(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_39(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_41(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_40(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_42(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_41(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_43(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_42(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_44(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_43(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_45(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_44(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_46(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_45(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_47(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_46(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_48(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_47(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_49(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_48(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_50(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_49(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_51(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_50(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_52(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_51(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_53(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_52(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_54(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_53(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_55(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_54(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_56(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_55(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_57(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_56(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_58(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_57(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_59(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_58(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_60(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_59(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_61(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_60(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_62(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_61(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_63(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_62(func, T, i + 1, __VA_ARGS__))
#define _REPEAT_64(func, T, i, arg, ...)  func(T, i, arg) _PARE(_REPEAT_63(func, T, i + 1, __VA_ARGS__))

#define _STR(x) #x
#define _CONCATE(x, y) x ## y
#define _STRING(x) _STR(x)
#define _PARE(...) __VA_ARGS__
#define _EAT(...)
#define _PAIR(x) _PARE x // PAIR((int) x) => PARE(int) x => int x
#define _STRIP(x) _EAT x // STRIP((int) x) => EAT(int) x => x
#define _PASTE(x, y) _CONCATE(x, y)

namespace sr {
	struct MetaInfo;

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
    decltype(::sr::_SymbolPrepare(                                  \
		[]() {                                                      \
		    struct Tmp {                                            \
	            constexpr static decltype(auto) get() noexcept {    \
	                return s;                                       \
	            }                                                   \
		    };                                                      \
	        return Tmp{};                                           \
	    } () )                                                      \
	)

    template<typename T>
    struct FieldCount {
	    constexpr static size_t value = 0;
    };

    template<typename T>
    constexpr static size_t FieldCountV = FieldCount<T>::value;

	template<typename T, size_t I>
	struct Field;


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

    // 动态反射, 主要用于序列化和反序列.
    // 动态反射的类型都是弱类型, 只区分 布尔, 整数, 浮点数, 字符串, 表,
    // 允许运行时通过 类型名字符串 查询发射信息
    enum DRType {
        Unknown,        // 未知的类型
        Boolean,        // bool
	    IntegerNumber,  // long
        RealNumber,     // double
        String,         // std::string
        Table,          // Table       
    };

    using DRBool = bool;
    using DRInt = long;
    using DRReal = double;
    using DRString = std::string;

    template<typename T>
    constexpr DRType GetDRType() {
        using RawType = std::decay_t<T>;
        if constexpr (std::is_same_v<RawType, bool>) {
			return DRType::Boolean;
        } else if constexpr (std::is_enum_v<RawType> || std::is_integral_v<RawType>) {
			return DRType::IntegerNumber;
        } else if constexpr (std::is_floating_point_v<RawType>) {
			return DRType::RealNumber;      
        } else if constexpr (std::is_same_v<RawType, char *>          ||
							 std::is_same_v<RawType, const char *>    ||
						     std::is_same_v<RawType, std::string>     || 
						     std::is_same_v<RawType, std::string_view> )
    	{
			return DRType::String;
        } else if constexpr (std::is_class_v<RawType>) {
            static_assert(FieldCountV<RawType> > 0, "This class does not register reflection");
	        return DRType::Table;
        } else {
            if constexpr (requires{ std::begin(std::declval<T>()); }) {
                DRType elementType = GetDRType< decltype(*std::begin(std::declval<T>())) >();
                static_assert(elementType != DRType::Unknown, "The elements of this container are not registered for reflection");
                return DRType::Table;
            }
            else if constexpr (std::is_array_v<T>) {
                DRType elementType = GetDRType< decltype(std::declval<T>()[0]) >();
                static_assert(elementType != DRType::Unknown, "The elements of this array are not registered for reflection");
                return DRType::Table;
            }
        }
        return DRType::Unknown;
    }

    template<typename T>
    constexpr DRType GetDRType(T &&) {
	    return GetDRType<T>();
    }

    struct MetaInfo {
        size_t index;
        DRType weakType;
	    std::string_view fieldName;
        std::string_view typeName;
        const std::type_info *pTypeInfo;
    };

    template<typename T>
    class DynamicRefl {
        using RawType = std::decay_t<T>;
        static inline std::unordered_map<std::string, size_t> sNameToIndexMap;
        static inline std::vector<MetaInfo> sMetaInfos;

        template<size_t I = 0>
        static void initialize() noexcept {
	        using FieldType = Field<RawType, I>;
            MetaInfo metaInfo = {
                .index = I,
                //.weakType = 
            };
        }
    public:
        DynamicRefl() {
	        
        }
    };


#define _FIELD_EACH(T, I, arg)                                              \
    template<>                                                              \
    struct ::sr::Field<T, I> {                                              \
        using type = decltype(std::declval<T>().arg);                       \
        consteval static std::string_view getName() noexcept {              \
            return _STR(arg);                                               \
        }                                                                   \
        constexpr static decltype(auto) getValue(T &obj) noexcept {         \
	        return (obj.arg);                                               \
        }                                                                   \
		constexpr static decltype(auto) getValue(const T &obj) noexcept {   \
			return (obj.arg);                                               \
		}                                                                   \
    };                                                              

#define _DEFINE_FIELD_COUNT(T, C)                               \
	template<>                                                  \
    struct ::sr::FieldCount<T> {                                \
		static constexpr size_t value = C;                      \
	}

#define DEFINE_REFLECT(T, ...)                                                      \
    _DEFINE_FIELD_COUNT(T, _GET_ARG_COUNT(__VA_ARGS__));                            \
    _PARE(_PASTE(_REPEAT_, _GET_ARG_COUNT(__VA_ARGS__)) (_FIELD_EACH, T, 0, __VA_ARGS__))  
}