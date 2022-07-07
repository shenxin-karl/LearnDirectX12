#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <string>
#include <vector>
#include <DirectXMath.h>
#include <iterator>
#include "ComponentStd.h"


namespace d3d {

namespace DX = DirectX;
namespace WRL = Microsoft::WRL;
using uint32 = std::uint32_t;
using uint64 = std::uint64_t;

class CameraBase;
class CoronaCamera;
class FirstPersonCamera;
struct LightData;
struct MaterialData;
struct CBPassType;
struct CBLightType;
class BlurFilter;
class SobelFilter;
class FXAA;
class Mesh;
class SkyBox;
class IBL;
class AssimpLoader;
class RenderTarget;
class RenderItem;
interface IModel;
interface ISubModel;
interface IMaterial;
interface IGeometryInput;
interface IInstanceInput;

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

WRL::ComPtr<ID3DBlob> compileShaderParseInclude(
	const char				*pFileName,
	const char				*fileContext,
	std::size_t				 sizeInByte,
	const D3D_SHADER_MACRO	*defines,
	const std::string		&entrypoint,
	const std::string		&target
);

class NonCopyable {
public:
	NonCopyable() = default;
	NonCopyable(const NonCopyable &) = delete;
	NonCopyable &operator=(const NonCopyable &) = delete;
	virtual ~NonCopyable() = default;
};

struct MacroPair {
	std::string key;
	std::string value;
};

std::string calcMacroKey(const std::string &name, const std::vector<D3D_SHADER_MACRO> &macros);
std::string calcMacroKey(const std::string &name, const D3D_SHADER_MACRO *pMacros, size_t size);

template<size_t N>
std::string calcMacroKey(const std::string &name, const D3D_SHADER_MACRO(&pMacros)[N]) {
	return calcMacroKey(name, pMacros, N);
}


void splitMacroKey(const std::string &key, std::string &name, std::vector<MacroPair> &macros);

class D3DInitializer {
	static inline std::atomic_bool isInited{ false };
public:
	D3DInitializer();
	~D3DInitializer();
	static void loadShaderDefineConfig();
};

}

namespace std {

std::string to_string(const std::wstring &wstr);
std::wstring to_wstring(const std::string &str);

}