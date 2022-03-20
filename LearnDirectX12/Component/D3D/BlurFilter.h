#pragma once
#include "dx12lib/dx12libStd.h"
#include "dx12lib/CommandListProxy.h"
#include "dx12lib/CommandContext.h"
#include "dx12lib/StructConstantBuffer.hpp"

namespace d3d {

class BlurFileter {
public:
	BlurFileter(dx12lib::ComputeContextProxy pComputeContext,
		std::uint32_t width,
		std::uint32_t height,
		DXGI_FORMAT format
	);

	void onResize(dx12lib::ComputeContextProxy pComputeList, std::uint32_t width, std::uint32_t height);

	template<typename T> requires(std::is_base_of_v<dx12lib::IShaderSourceResource, T>)
	void produce(dx12lib::ComputeContextProxy pComputeList,
		std::shared_ptr<T> pShaderResource, 
		int blurCount,
		float sigma = 1.f)	
	{
		this->produceImpl(pComputeList, pShaderResource, blurCount);
	}

	void produceImpl(dx12lib::ComputeContextProxy pComputeList,
		std::shared_ptr<dx12lib::IShaderSourceResource> pShaderResource,
		int blurCount,
		float sigma
	);

	std::shared_ptr<dx12lib::UnorderedAccessBuffer> getOuput() const;
private:
	void buildUnorderedAccessResouce(dx12lib::ComputeContextProxy pComputeContext);
	std::vector<float> calcGaussianWeights(int blurCount);

	constexpr static std::size_t kMaxBlurCount = 5;
	struct BlurCBType {
		int   blurCount;
		float weights[kMaxBlurCount * 2 + 1];
	};
private:
	std::uint32_t _width;
	std::uint32_t _height;
	DXGI_FORMAT   _format;
	GPUStructCBPtr<BlurCBType> _pBlurCB;
	std::shared_ptr<dx12lib::ComputePSO> _pHorzBlurPSO;
	std::shared_ptr<dx12lib::ComputePSO> _pVertBlurPSO;
	std::shared_ptr<dx12lib::UnorderedAccessBuffer> _pBlurMap0;
	std::shared_ptr<dx12lib::UnorderedAccessBuffer> _pBlurMap1;
};

}