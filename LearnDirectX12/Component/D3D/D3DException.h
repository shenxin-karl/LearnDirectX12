#pragma once
#include "Exception/ExceptionBase.h"
#define  NOMINMAX
#include <Windows.h>

namespace d3d {

class D3DException : public com::ExceptionBase {
public:
	D3DException(HRESULT hr, const char *file, int line);
	virtual const char *what() const noexcept override;
	virtual const char *getType() const noexcept;
	std::string getErrorString() const;
public:
	HRESULT hr_;
};

}