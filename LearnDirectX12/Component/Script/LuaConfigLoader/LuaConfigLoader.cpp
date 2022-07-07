#include "LuaConfigLoader.h"
#include <cassert>
#include <iostream>

namespace scr {

LuaConfigLoader::LuaConfigLoader(const std::string &fileName) : _fileName(fileName), _pLuaState(luaL_newstate()) {
	luaL_openlibs(_pLuaState);
	if (luaL_loadfile(_pLuaState, _fileName.c_str()) || lua_pcall(_pLuaState, 0, 0, 0)) {
		std::cerr << lua_tostring(_pLuaState, -1) << std::endl;
		lua_close(_pLuaState);
		_pLuaState = nullptr;
		return;
	}
}

LuaConfigLoader::~LuaConfigLoader() {
	if (_pLuaState != nullptr) {
		lua_close(_pLuaState);
		_pLuaState = nullptr;
	}
}

bool LuaConfigLoader::isLoad() const {
	return _pLuaState != nullptr;
}

auto LuaConfigLoader::getString(const std::string &key) -> std::optional<std::string> {
	getKey(key);
	return getString();
}

auto LuaConfigLoader::getBoolean(const std::string &key) -> std::optional<bool> {
	getKey(key);
	return getBoolean();
}

auto LuaConfigLoader::getNumber(const std::string &key) -> std::optional<double> {
	getKey(key);
	return getNumber();
}

std::string LuaConfigLoader::tryGetString(const std::string &key, const std::string &defString) {
	if (auto str = getString(key))
		return *str;
	return defString;
}

auto LuaConfigLoader::tryGetBoolean(const std::string &key, bool defBool) -> bool {
	if (auto bCond = getBoolean(key))
		return *bCond;
	return defBool;
}

auto LuaConfigLoader::tryGetNumber(const std::string &key, double defNumber) -> double {
	if (auto number = getNumber(key))
		return *number;
	return defNumber;
}

bool LuaConfigLoader::isString(const std::string &key) const {
	getKey(key);
	bool result = lua_isstring(_pLuaState, -1);
	lua_pop(_pLuaState, 1);
	return result;
}

bool LuaConfigLoader::isNumber(const std::string &key) const {
	getKey(key);
	bool result = lua_isnumber(_pLuaState, -1);
	lua_pop(_pLuaState, 1);
	return result;
}

bool LuaConfigLoader::isBoolean(const std::string &key) const {
	getKey(key);
	bool result = lua_isboolean(_pLuaState, -1);
	lua_pop(_pLuaState, 1);
	return result;
}

bool LuaConfigLoader::isTable(const std::string &key) const {
	getKey(key);
	bool result = lua_istable(_pLuaState, -1);
	lua_pop(_pLuaState, 1);
	return result;
}

bool LuaConfigLoader::beginTable(const std::string &key) {
	getKey(key);
	bool isTable = lua_istable(_pLuaState, -1);
	++_numNested;
	return isTable;
}

void LuaConfigLoader::endTable() {
	bool isTable = lua_istable(_pLuaState, -1);
	assert(isTable);
	lua_pop(_pLuaState, 1);
	--_numNested;
}

std::variant<std::monostate, std::string, double> LuaConfigLoader::getKey() {
	if (lua_isstring(_pLuaState, -2))
		return std::string(lua_tostring(_pLuaState, -2));
	else if (lua_isnumber(_pLuaState, -2))
		return lua_tonumber(_pLuaState, -2);
	return {};
}

void LuaConfigLoader::beginNext() {
	lua_pushnil(_pLuaState);
}

bool LuaConfigLoader::next() {
	if (lua_next(_pLuaState, -1)) {
		lua_pushvalue(_pLuaState, -2);
		return true;
	}
	return false;
}

LuaValueType LuaConfigLoader::getValueType() const {
	return static_cast<LuaValueType>(lua_type(_pLuaState, -1));
}

auto LuaConfigLoader::getString() -> std::optional<std::string> {
	std::optional<std::string> result;
	if (lua_isboolean(_pLuaState, -1))
		result = lua_tostring(_pLuaState, -1);
	lua_pop(_pLuaState, 1);
	return result;
}

std::optional<bool> LuaConfigLoader::getBoolean() {
	std::optional<bool> result;
	if (lua_isboolean(_pLuaState, -1))
		result = lua_toboolean(_pLuaState, -1);
	lua_pop(_pLuaState, 1);
	return result;
}

std::optional<double> LuaConfigLoader::getNumber() {
	std::optional<double> result;
	if (lua_isnumber(_pLuaState, -1))
		result = lua_tonumber(_pLuaState, -1);
	lua_pop(_pLuaState, 1);
	return result;
}

auto LuaConfigLoader::tryGetString(const std::string &defKey) -> std::string {
	if (auto str = getString())
		return *str;
	return defKey;
}

auto LuaConfigLoader::tryGetBoolean(bool defBool) -> bool {
	if (auto bCond = getBoolean())
		return *bCond;
	return defBool;
}

auto LuaConfigLoader::tryGetNumber(double defNumber) -> double {
	if (auto number = getNumber())
		return *number;
	return defNumber;
}

bool LuaConfigLoader::beginTable() {
	++_numNested;
	return getValueType() == LuaValueType::Table;
}

void LuaConfigLoader::discard() {
	lua_pop(_pLuaState, 1);
}

size_t LuaConfigLoader::getTableLength() const {
	return luaL_len(_pLuaState, -1);
}

void LuaConfigLoader::getKey(const std::string &key) const {
	if (_numNested == 0)
		std::cout << lua_getglobal(_pLuaState, key.c_str()) << std::endl;
	else
		lua_getfield(_pLuaState, -1, key.c_str());
}

}
