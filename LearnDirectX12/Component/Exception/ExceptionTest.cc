#include <iostream>
#include <format>
#define NOMINMAX
#include <Windows.h>
#include "ExceptionBase.h"

class TestException : public com::ExceptionBase {
public:
	TestException(int errNum, const char *file, int line) : ExceptionBase(file, line), errNum_(errNum) {}
	virtual const char *what() const noexcept override {
		ExceptionBase::what();
		whatBuffer_ += std::format("error number: {}", errNum_);
		return whatBuffer_.c_str();
	}

	virtual const char *getType() const noexcept override {
		return "TestExcepthion";
	}
public:
	int errNum_ = 0;
};


int main(void) {
	try {
		throw TestException(-1, __FILE__, __LINE__);
	} catch (const com::ExceptionBase &e) {
		std::cerr << e.what() << std::endl;
		OutputDebugStringA(e.what());
	}
	return 0;
}