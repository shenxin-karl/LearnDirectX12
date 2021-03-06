#pragma once
#include <dx12lib/dx12libStd.h>
#include <dx12lib/Context/ContextStd.h>
#include <dx12lib/Resource/ResourceStd.h>

namespace d3d {

class BlurFilter {
public:
	BlurFilter(dx12lib::ComputeContextProxy pComputeContext,
		std::uint32_t width,
		std::uint32_t height,
		DXGI_FORMAT format
	);

	void onResize(dx12lib::ComputeContextProxy pComputeList, std::uint32_t width, std::uint32_t height);

	template<typename T> 
	void produce(dx12lib::ComputeContextProxy pComputeList,
		std::shared_ptr<T> pShaderResource, 
		int blurCount,
		float sigma = 1.f)	
	{
		this->produceImpl(pComputeList, pShaderResource, blurCount, sigma);
	}

	void produceImpl(dx12lib::ComputeContextProxy pComputeList,
		std::shared_ptr<dx12lib::IResource> pResource,
		int blurCount,
		float sigma
	);

	std::shared_ptr<dx12lib::UnorderedAccess2D> getOutput() const;
private:
	void buildUnorderedAccessResource(dx12lib::ComputeContextProxy pComputeContext);
	static void buildBlurPSO(std::weak_ptr<dx12lib::Device> pDevice);
	static std::vector<float> calcGaussianWeights(int blurCount, float sigma);
	static std::size_t getBlurRadiusBySigma(float sigma);
	void updateBlurConstantBuffer(dx12lib::ComputeContextProxy pComputeList, int blurCount, float sigma);

	constexpr static std::size_t kMaxBlurCount = 5;
	constexpr static std::size_t kMaxBlurRadius = 5;
	constexpr static std::size_t kMaxThreads = 256;
	struct BlurCBType {
		int   blurCount;
		float weights[kMaxBlurCount * 2 + 1];
	};
private:
	std::uint32_t _width;
	std::uint32_t _height;
	DXGI_FORMAT   _format = DXGI_FORMAT_R8G8B8A8_UNORM;
	std::shared_ptr<dx12lib::UnorderedAccess2D> _pBlurMap0;
	std::shared_ptr<dx12lib::UnorderedAccess2D> _pBlurMap1;
	static inline std::shared_ptr<dx12lib::ComputePSO> _pHorzBlurPSO;
	static inline std::shared_ptr<dx12lib::ComputePSO> _pVertBlurPSO;
};

}