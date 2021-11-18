//#include <format>
#include <sstream>
#include "ExceptionBase.h"

namespace com {

ExceptionBase::ExceptionBase(const char *file, int line) : file_(file), line_(line) {
}

const char *ExceptionBase::what() const noexcept {
	std::stringstream sbuf;
	sbuf << getType() << std::endl << getOriginString();
	//whatBuffer_ = std::format("{}\n{}", getType(), getOriginString());
	whatBuffer_ = sbuf.str();
	return whatBuffer_.c_str();
}

const char *ExceptionBase::getType() const noexcept {
	return "ExceptionBase";
}

int ExceptionBase::getLine() const noexcept {
	return line_;
}

const std::string &ExceptionBase::getFile() const noexcept {
	return file_;
}

std::string ExceptionBase::getOriginString() const noexcept {
	//return std::format("[File] {}\n[Line] {}\n", file_, line_);
	std::stringstream sbuf;
	sbuf << "[File]: " << file_ << std::endl << "[Line]: " << line_ << std::endl;
	return sbuf.str();
}

}