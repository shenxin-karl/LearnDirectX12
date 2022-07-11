#pragma once
#include <string>
#include <optional>
#include <variant>
#include <source_location>

extern "C" {

#include <lua/lua.h>
#include <lua/lauxlib.h>
#include <lua/lualib.h>
}

namespace scr {

enum class LuaValueType {
	Nil      = LUA_TNIL,
	Boolean  = LUA_TBOOLEAN,
	String   = LUA_TSTRING,
	Number   = LUA_TNUMBER,
	Table    = LUA_TTABLE,
	UserData = LUA_TUSERDATA,
	Function = LUA_TFUNCTION,
	Thread   = LUA_TTHREAD,
	NumTypes = LUA_NUMTYPES,
};

class LuaConfigLoader {
public:
	LuaConfigLoader(const std::string &fileName);
	LuaConfigLoader(const void *pData, size_t sizeInByte, const std::source_location &sr = std::source_location::current());
	~LuaConfigLoader();
	bool isLoad() const;
	auto tryGetString(const std::string &key) -> std::optional<std::string>;
	auto tryGetBoolean(const std::string &key) -> std::optional<bool>;
	auto tryGetNumber(const std::string &key) -> std::optional<double>;
	auto getString(const std::string &key, const std::string &defString = {}) -> std::string;
	auto getBoolean(const std::string &key, bool defBool = false) -> bool;
	auto getNumber(const std::string &key, double defNumber = 0.f) -> double;
	bool isString(const std::string &key) const;
	bool isNumber(const std::string &key) const;
	bool isBoolean(const std::string &key) const;
	bool isTable(const std::string &key) const;
	bool beginTable(const std::string &key);
	void endTable();
	LuaValueType getValueType() const;
	auto tryGetString() -> std::optional<std::string>;
	auto tryGetBoolean() -> std::optional<bool>;
	auto tryGetNumber() -> std::optional<double>;
	auto getString(const std::string &defKey = {}) -> std::string;
	auto getBoolean(bool defBool = false) -> bool;
	auto getNumber(double defNumber = 0.0) -> double;
	bool beginTable();
	void discard();
	size_t getTableLength() const;
	struct NextVisitor;
	NextVisitor next();
private:
	void getKey(const std::string &key) const;
private:
	size_t _numNested = 0;
	mutable lua_State *_pLuaState;
};

struct LuaConfigLoader::NextVisitor {
	struct EndTag {};
	struct TableKey {
		double numKey;
		std::string strKey;
	};
	struct Iterator {
		TableKey operator*();
		Iterator &operator++();
		friend bool operator!=(const Iterator &, const EndTag &);
	private:
		friend struct NextVisitor;
		int _index;
		mutable lua_State *_pLuaState;
		mutable TableKey _tableKey;
		mutable bool _shouldNext  : 1 = true;		// 是否可以下一次迭代
		mutable bool _firstIter   : 1 = true;		// 是否第一次迭代
	};
public:
	explicit NextVisitor(lua_State *pLuaState);
	Iterator begin();
	EndTag end();
private:
	lua_State *_pLuaState;
};

}
