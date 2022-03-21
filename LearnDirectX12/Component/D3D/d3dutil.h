#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <string>
#include "ComponentStd.h"


namespace d3d {

namespace WRL = Microsoft::WRL;
using uint32 = std::uint32_t;
using uint64 = std::uint64_t;

class CameraBase;
class CoronaCamera;
struct Light;
struct Material;
struct PassCBType;
struct LightCBType;
class BlurFilter;

using ::ThrowIfFailed;

WRL::ComPtr<ID3DBlob> compileShader(
	const std::wstring		&fileName,
	const D3D_SHADER_MACRO  *defines,
	const std::string		&entrypoint,
	const std::string		&target
);

WRL::ComPtr<ID3DBlob> compileShader(
	const char				*fileContext,
	std::size_t				 sizeInByte,
	const D3D_SHADER_MACRO  *defines,
	const std::string		&entrypoint,
	const std::string		&target
);

}