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
	auto getString(const std::string &key) -> std::optional<std::string>;
	auto getBoolean(const std::string &key) -> std::optional<bool>;
	auto getNumber(const std::string &key) -> std::optional<double>;
	auto tryGetString(const std::string &key, const std::string &defString = {}) -> std::string;
	auto tryGetBoolean(const std::string &key, bool defBool = false) -> bool;
	auto tryGetNumber(const std::string &key, double defNumber = 0.f) -> double;
	bool isString(const std::string &key) const;
	bool isNumber(const std::string &key) const;
	bool isBoolean(const std::string &key) const;
	bool isTable(const std::string &key) const;
	bool beginTable(const std::string &key);
	void endTable();
	LuaValueType getValueType() const;
	auto getString() -> std::optional<std::string>;
	auto getBoolean() -> std::optional<bool>;
	auto getNumber() -> std::optional<double>;
	auto tryGetString(const std::string &defKey = {}) -> std::string;
	auto tryGetBoolean(bool defBool = false) -> bool;
	auto tryGetNumber(double defNumber = 0.0) -> double;
	bool beginTable();
	void discard();
	size_t getTableLength() const;
	struct TableKey;
	TableKey getKey();
	void beginNext();
	bool next();
private:
	void getKey(const std::string &key) const;
private:
	size_t _numNested = 0;
	mutable lua_State *_pLuaState;
};

struct LuaConfigLoader::TableKey {
public:
	TableKey() = default;
	explicit TableKey(const std::string &str);
	explicit TableKey(double num);
	TableKey &operator=(const TableKey &) = default;
	const std::string &toString() const;
	double toNumber() const;
	explicit operator bool() const;
private:
	std::string _strKey;
	double _numKey;
};

}
