#include <windows.h>
#include "D3Dx12.h"
#include "Graphics.h"

void _ThrowIfFailedImpl(const char *file, int line, HRESULT hr) {
	if (FAILED(hr))
		throw GraphicsException(file, line, hr);
	return;
}
