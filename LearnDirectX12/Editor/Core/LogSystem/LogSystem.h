#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <Singleton/Singleton.hpp>

namespace core {

class LogSystem : public com::Singleton<LogSystem> {
public:
	explicit LogSystem(const std::string &fileName);
	virtual void append(const std::string &msg);
	const std::string &getFileName() const;
private:
	std::ofstream _file;
	std::string _fileName;
	std::atomic_flag _lock;
};

}
