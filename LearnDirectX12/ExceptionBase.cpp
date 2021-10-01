#include <format>
#include "ExceptionBase.h"

ExceptionBase::ExceptionBase(const char *file, int line) : file_(file), line_(line) {
}

const char *ExceptionBase::what() const noexcept {
	whatBuffer_ = std::format("{}\n{}", getType(), getOriginString());
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
	return std::format("[File] {}\n[Line] {}\n", file_, line_);
}
