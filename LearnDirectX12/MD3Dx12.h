#pragma once
#include <winnt.h>

void _ThrowIfFailedImpl(const char *file, int line, HRESULT hr);
#define ThrowIfFailed(hr) (_ThrowIfFailedImpl(__FILE__, __LINE__, hr))