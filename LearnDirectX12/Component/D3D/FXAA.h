#pragma once
#include "dx12lib/dx12libStd.h"
#include "dx12lib/ContextProxy.hpp"
#include "d3dUtil.h"

namespace d3d {
	
class FXAA : public NonCopyable {
public:
	FXAA(dx12lib::ComputeContextProxy pComputeCtx,
		std::uint32_t width, 
		std::uint32_t height, 
		DXGI_FORMAT format
	);

	template<typename T> requires(std::is_base_of_v<dx12lib::IShaderSourceResource, T>)
	void produce(dx12lib::ComputeContextProxy pComputeCtx, std::shared_ptr<T> pInput) const {
		_produceImpl(
			pComputeCtx,
			std::static_pointer_cast<dx12lib::IShaderSourceResource>(pInput)
		);
	}

	void _produceImpl(dx12lib::ComputeContextProxy pComputeCtx, 
		std::shared_ptr<dx12lib::IShaderSourceResource> pInput
	) const;

	void onResize(dx12lib::ComputeContextProxy pComputeCtx, uint32 width, uint32 height);
	std::shared_ptr<dx12lib::UnorderedAccessBuffer> getOutput() const;
public:
	float _minThreshold = 0.0312f;
	float _threshold = 0.125f;
	float _consoleTangentScale = 0.7f;
	float _gSharpness = 8.f;
private:
	enum RootParame {
		CB_Setting,
		SR_Input,
		UA_Output,
	};
	static void tryBuildRootSignature(std::weak_ptr<dx12lib::Device> pDevice);
	static void tryBuildConsolePSO(std::weak_ptr<dx12lib::Device> pDevice);
	void updateFXAASetting(dx12lib::ComputeContextProxy pComputeCtx) const;
	constexpr static std::size_t kFXAAThreadCount = 16;
private:
	std::uint32_t _width;
	std::uint32_t _height;
	DXGI_FORMAT   _format;
	std::shared_ptr<dx12lib::UnorderedAccessBuffer> _pOutputMap;
	static inline std::shared_ptr<dx12lib::ComputePSO>	_pConsolePSO;
	static inline std::shared_ptr<dx12lib::RootSignature> _pRootSingnature;
};

}