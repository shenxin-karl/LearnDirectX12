#include <comdef.h>
#include "D3DException.h"

namespace d3d {

D3DException::D3DException(HRESULT hr, const char *file, int line)
	: hr_(hr), ExceptionBase(file, line) {}

const char *D3DException::what() const noexcept {
	ExceptionBase::what();
	whatBuffer_ += getErrorString();
	return whatBuffer_.c_str();
}

const char * D3DException::getType() const noexcept {
	return "D3DException";
}

std::string D3DException::getErrorString() const {
	_com_error err(hr_);
	std::string msg = err.ErrorMessage();
	return msg;
}

}