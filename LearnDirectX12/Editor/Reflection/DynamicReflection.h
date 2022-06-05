#pragma once
#include <string>
#include <typeinfo>
#include <unordered_map>
#include "Reflection/StaticReflection.hpp"

namespace refl {

struct MetaInfo;

// 动态反射, 主要用于序列化和反序列.
// 动态反射的类型都是弱类型, 只区分 布尔, 整数, 浮点数, 字符串, 表,
// 允许运行时通过 类型名字符串 查询发射信息
enum class DRType {
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
consteval DRType toDRType() {
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
            constexpr DRType elementType = toDRType< decltype(*std::begin(std::declval<T>())) >();
            static_assert(elementType != DRType::Unknown, "The elements of this container are not registered for reflection");
            return DRType::Table;
        }
        else if constexpr (std::is_array_v<T>) {
            constexpr DRType elementType = toDRType< decltype(std::declval<T>()[0]) >();
            static_assert(elementType != DRType::Unknown, "The elements of this array are not registered for reflection");
            return DRType::Table;
        }
    }
    return DRType::Unknown;
}


template<typename T>
consteval DRType toDRType(T &&) {
    return toDRType<T>();
}

struct TableType {};
struct UnknownType {};

template<DRType DR>
constexpr auto formDRType() {
	if constexpr (DR == DRType::Boolean)
		return DRBool();
    else if constexpr (DR == DRType::IntegerNumber)
		return DRInt();
    else if constexpr (DR == DRType::RealNumber)
		return DRReal();
    else if constexpr (DR == DRType::String)
		return DRString();
    else if constexpr (DR == DRType::Table)
        return TableType();

    static_assert(DR != Unknown, "Encountered a type that could not reflect");
    return UnknownType();
}

struct TransformFunc {
	void *pGetFunc = nullptr;
    void *pSetFunc = nullptr;
public:
    template<typename DstType, typename SrcType>
    static void setFunction(void *pData, const DstType &data) noexcept {
	    *static_cast<SrcType *>(pData) =  static_cast<DstType>(data);
    }

    template<typename DstType, typename SrcType>
    static const DstType &getFunction(void *pData) noexcept {
        return static_cast<const DstType &>(*static_cast<SrcType *>(pData));
    }
};

class Table {
private:
    void *_pTable = nullptr;

};

class DRValue {
public:
    auto toBool() const -> DRBool;
    auto toInt() const -> DRInt;
    auto toReal() const -> DRReal;
    auto toString() const -> const DRString &;
    auto toTable() const -> const Table &;
    auto isConstant() const;
    explicit operator const DRBool &() const;
    explicit operator const DRInt &() const;
    explicit operator const DRReal &() const;
    explicit operator const DRString &() const;
    explicit operator const Table &() const;
    DRValue &operator=(DRBool vBool);
    DRValue &operator=(DRInt vInt);
    DRValue &operator=(DRReal vReal);
    DRValue &operator=(const DRString &vString);
private:
    template<typename T>
    DRValue(T &&object, MetaInfo *pMetaInfo, TransformFunc transformFunc);
private:
    bool _isConstant = false;
    void *_pData = nullptr;
    MetaInfo *_pMetaInfo = nullptr;
	TransformFunc _transformFunc;
};

struct MetaInfo {
    size_t index;
    DRType weakType;
    std::string_view fieldName;
    std::string_view typeName;
    const std::type_info *pTypeInfo;
};

template<typename T>
class DynamicReflItem {
    using RawType = std::decay_t<T>;
    constexpr static size_t sFiledCount = FieldCountV<RawType>;
    inline static std::array<MetaInfo, sFiledCount> sMetaInfos;
    inline static std::array<TransformFunc, sFiledCount> sTransformFunc;
    inline static std::unordered_map<std::string, size_t> sNameToIndexMap;

    template<size_t I = 0>
    static void initialize() noexcept {
        using FieldType = Field<RawType, I>;
        if constexpr (I < sFiledCount) {
            constexpr DRType drType = toDRType< typename Field<RawType, I>::type >();
            using DstType = decltype(formDRType<drType>());
            using SrcType = typename Field<RawType, I>::type;

            const std::type_info &typeInfo = typeid(SrcType);
            MetaInfo metaInfo = {
                .index = I,
                .weakType = drType,
                .fieldName = FieldType::getName(),
                .typeName = typeInfo.name(),
                .pTypeInfo = &typeInfo,
            };

            std::string fieldName = FieldType::getName().data();
            sNameToIndexMap[fieldName] = I;
            sMetaInfos[I] = metaInfo;
            if constexpr (!std::is_same_v<DstType, TableType>) {
            	void *pGetFunc = &TransformFunc::getFunction<DstType, SrcType>;
                void *pSetFunc = &TransformFunc::setFunction<DstType, SrcType>;
				sTransformFunc[I].pGetFunc = pGetFunc;
            	sTransformFunc[I].pSetFunc = pSetFunc;
            }
            initialize<I+1>();
        }
    }
public:
    DynamicReflItem() {
        initialize();
    }
};



}
