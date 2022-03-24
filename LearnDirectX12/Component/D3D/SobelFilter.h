#pragma once
#include "dx12lib/dx12libStd.h"
#include "dx12lib/CommandListProxy.h"
#include "dx12lib/CommandContext.h"
#include "dx12lib/StructConstantBuffer.hpp"
#include "dx12lib/IResource.h"

namespace d3d {

class SobelFilter {
public:
	SobelFilter(dx12lib::ComputeContextProxy pComputeContext, std::uint32_t width, std::uint32_t height);
	void onResize(dx12lib::ComputeContextProxy pComputeList, std::uint32_t width, std::uint32_t height);

	template<typename T> requires(std::is_base_of_v<dx12lib::IShaderSourceResource, T>)
	void produce(dx12lib::ComputeContextProxy pComputeList, std::shared_ptr<T> pInput) {
		this->produceImpl(pComputeList, pInput);
	}

	template<typename T> requires(std::is_base_of_v<dx12lib::IShaderSourceResource, T>)
	void apply(dx12lib::ComputeContextProxy pComputeList, std::shared_ptr<T> pInput) {
		this->applyImpl(pComputeList, pInput);
	}

	std::shared_ptr<dx12lib::UnorderedAccessBuffer> getOutput() const;
private:
	void tryBuildSobelMap(dx12lib::ComputeContextProxy pComputeList, DXGI_FORMAT format);
	static void tryBuildRootSignature(dx12lib::ComputeContextProxy pComputeList);
	static void tryBuildProducePSO(dx12lib::ComputeContextProxy pComputeList);
	static void tryBuildApplyPSO(dx12lib::ComputeContextProxy pComputeList);
	void produceImpl(dx12lib::ComputeContextProxy pComputeList, 
		std::shared_ptr<dx12lib::IShaderSourceResource> pInput
	);
	void applyImpl(dx12lib::ComputeContextProxy pComputeList,
		std::shared_ptr<dx12lib::IShaderSourceResource> pInput
	);

	enum SobelRootParame {
		SR_Input  = 0,
		UA_Output = 1,
	};

private:
	constexpr static std::size_t kMaxSobelThreadCount = 16;
	std::uint32_t _width;
	std::uint32_t _height;
	std::shared_ptr<dx12lib::UnorderedAccessBuffer> _pSobelMap;
	static inline std::shared_ptr<dx12lib::ComputePSO> _pProducePSO;
	static inline std::shared_ptr<dx12lib::ComputePSO> _pAllpyPSO;
	static inline std::shared_ptr<dx12lib::RootSignature> _pRootSignature;
};

}