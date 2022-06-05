#include <cassert>
#include "LogSystem.h"

namespace core {

LogSystem::LogSystem(const std::string &fileName) : _file(fileName, std::ios::out), _fileName(fileName) {
	assert(_file.is_open());
}

void LogSystem::append(const std::string &msg) {
	while (_lock.test_and_set())
		_lock.wait(false);

	_file << msg;

	_lock.clear();
	_lock.notify_one();
}

const std::string &LogSystem::getFileName() const {
	return _fileName;
}

}
