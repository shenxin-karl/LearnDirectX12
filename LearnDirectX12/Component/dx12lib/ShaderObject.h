#pragma once
#include "dx12libStd.h"
#include "CommandListProxy.h"

namespace com {
class GameTimer;
}

namespace dx12lib {

template<typename T, typename...Args> requires(std::is_base_of_v<ShaderObject, T>)
std::shared_ptr<T> createShaderObject(Args&&... args) {
#define ErrorReturnNullptr(b) do { if (!b) return nullptr; } while (false)
	try {
		auto pShaderObject = std::make_shared<T>(std::forward<Args>(args)...);
		ErrorReturnNullptr(pShaderObject->loadShader());
		ErrorReturnNullptr(pShaderObject->onBuildInputLayout());
		ErrorReturnNullptr(pShaderObject->onBuildRootSignature());
		ErrorReturnNullptr(pShaderObject->onBuildPipelineStateObject());
		return pShaderObject;
	} catch (...) {
		throw;
	}
#undef ErrorReturnNullptr
}

class ShaderObject {
public:
	template<typename T, typename...Args>
	friend std::shared_ptr<T> createShaderObject(Args&&... args);
// initialize step
	virtual bool onBuildInputLayout() { return true; }
	virtual bool onBuildShader() { return true; }
	virtual bool onBuildRootSignature() { return true; }
	virtual bool onBuildPipelineStateObject() { return true; }
// do draw
	virtual void activate(CommandListProxy pCmdList) {}
protected:
	bool loadShader();
	ShaderObject(const std::string &shaderFile);
	ShaderObject(const std::string &shaderFile, WRL::ComPtr<ID3DBlob> pShaderCode);
	ShaderObject(const ShaderObject &) = delete;
protected:
	void compileShader(WRL::ComPtr<ID3DBlob> &pByteCode, 
		const std::string &entrypoint,
		const std::string &target,
		const D3D_SHADER_MACRO *defines = nullptr
	);
protected:
	std::string _shaderFile;
	WRL::ComPtr<ID3DBlob> _pShaderCode;
	WRL::ComPtr<ID3DBlob> _pVSByteCode;
	WRL::ComPtr<ID3DBlob> _pFSByteCode;
	WRL::ComPtr<ID3DBlob> _pHSByteCode;
	WRL::ComPtr<ID3DBlob> _pDSByteCode;
	WRL::ComPtr<ID3DBlob> _pGSByteCode;
	WRL::ComPtr<ID3DBlob> _pCSByteCode;
	std::shared_ptr<RootSignature> _pRootSignature;
	std::shared_ptr<PipelineStateObject> _pPSO;
};

}