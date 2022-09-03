#include "BlurFilter.h"
#include "D3D/d3dutil.h"
#include "D3D/Shader/D3DShaderResource.h"
#include <dx12lib/Texture/TextureStd.h>
#include <dx12lib/Pipeline/PipelineStd.h>


namespace d3d {

BlurFilter::BlurFilter(dx12lib::ComputeContextProxy pComputeContext,
	std::uint32_t width, 
	std::uint32_t height, 
	DXGI_FORMAT format) 
: _width(width), _height(height), _format(format)
{
	assert(format != DXGI_FORMAT_UNKNOWN);
	assert(width > 0);
	assert(height > 0);
	buildUnorderedAccessResource(pComputeContext);
	buildBlurPSO(pComputeContext->getDevice());
}

void BlurFilter::onResize(dx12lib::ComputeContextProxy pComputeList, std::uint32_t width, std::uint32_t height) {
	if (_width != width || _height != height) {
		_width = width;
		_height = height;
		buildUnorderedAccessResource(pComputeList);
	}
}

void BlurFilter::produceImpl(dx12lib::ComputeContextProxy pComputeList,
	std::shared_ptr<dx12lib::IResource> pResource,
	int blurCount,
	float sigma)
{
	auto weights = calcGaussianWeights(blurCount, sigma);
	auto blurRadius = getBlurRadiusBySigma(sigma);
	auto updateConstantBuffer = [&]() {
		pComputeList->setCompute32BitConstants(dx12lib::RegisterSlot::CBV0, 1, &blurRadius);
		pComputeList->setCompute32BitConstants(dx12lib::RegisterSlot::CBV0, 11, weights.data(), 1);
	};

	pComputeList->copyResource(_pBlurMap1, pResource);
	for (int i = 0; i < blurCount; ++i) {
		// horizonal blur
		pComputeList->setComputePSO(_pHorzBlurPSO);
		pComputeList->transitionBarrier(_pBlurMap1, D3D12_RESOURCE_STATE_GENERIC_READ);
		pComputeList->transitionBarrier(_pBlurMap0, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		pComputeList->setShaderResourceView(dx12lib::RegisterSlot::SRV0, _pBlurMap1->getSRV());
		pComputeList->setUnorderedAccessView(dx12lib::RegisterSlot::UAV0, _pBlurMap0->getUAV());
		updateConstantBuffer();
		int numXGroup = static_cast<int>(std::ceil(_width / static_cast<float>(kMaxThreads)));
		pComputeList->dispatch(numXGroup, _height, 1);
		
		// vertical blur
		pComputeList->setComputePSO(_pVertBlurPSO);
		pComputeList->transitionBarrier(_pBlurMap0, D3D12_RESOURCE_STATE_GENERIC_READ);
		pComputeList->transitionBarrier(_pBlurMap1, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		pComputeList->setShaderResourceView(dx12lib::RegisterSlot::SRV0, _pBlurMap0->getSRV());
		pComputeList->setUnorderedAccessView(dx12lib::RegisterSlot::UAV0, _pBlurMap1->getUAV());
		updateConstantBuffer();
		int numYGroup = static_cast<int>(std::ceil(_height / static_cast<float>(kMaxThreads)));
		pComputeList->dispatch(_width, numYGroup, 1);
	}
}

std::shared_ptr<dx12lib::UnorderedAccess2D> BlurFilter::getOutput() const {
	return _pBlurMap1;
}

void BlurFilter::buildUnorderedAccessResource(dx12lib::ComputeContextProxy pComputeContext) {
	_pBlurMap0 = pComputeContext->createUnorderedAccess2D(_width, _height, _format);
	_pBlurMap1 = pComputeContext->createUnorderedAccess2D(_width, _height, _format);
}

void BlurFilter::buildBlurPSO(std::weak_ptr<dx12lib::Device> pDevice) {
	if (_pHorzBlurPSO == nullptr) {
		auto pBlurFilterCSFile = getD3DResource("HlslShader/BlurFilterCS.hlsl");
		assert(pBlurFilterCSFile.size() != 0);

		auto pSharedDevice = pDevice.lock();
		auto pRootSignature = pSharedDevice->createRootSignature(2);
		pRootSignature->at(0).initAsConstants(dx12lib::RegisterSlot::CBV0, 12);
		pRootSignature->at(1).initAsDescriptorTable({
			{ dx12lib::RegisterSlot::SRV0, 1 },
			{ dx12lib::RegisterSlot::UAV0, 1 },
		});
		pRootSignature->finalize();

		_pHorzBlurPSO = pSharedDevice->createComputePSO("HorizontalBlurPSO");
		_pVertBlurPSO = pSharedDevice->createComputePSO("VerticalBlurPSO");

		_pHorzBlurPSO->setRootSignature(pRootSignature);
		_pVertBlurPSO->setRootSignature(pRootSignature);
		_pHorzBlurPSO->setComputeShader(compileShader(
			pBlurFilterCSFile.begin(),
			pBlurFilterCSFile.size(),
			nullptr, 
			"HorizonBlurCS",
			"cs_5_0"
		));
		_pVertBlurPSO->setComputeShader(compileShader(
			pBlurFilterCSFile.begin(),
			pBlurFilterCSFile.size(),
			nullptr,
			"VerticalBlurCS",
			"cs_5_0"
		));

		_pHorzBlurPSO->finalize();
		_pVertBlurPSO->finalize();
	}
}

void BlurFilter::updateBlurConstantBuffer(dx12lib::ComputeContextProxy pComputeList, int blurCount, float sigma) {
	std::vector<float> weights = calcGaussianWeights(blurCount, sigma);
	pComputeList->setCompute32BitConstants(dx12lib::RegisterSlot::CBV0, 1, &blurCount);
	pComputeList->setCompute32BitConstants(dx12lib::RegisterSlot::CBV0, 11, weights.data(), 1);
}

std::vector<float> BlurFilter::calcGaussianWeights(int blurCount, float sigma) {
	std::vector<float> weights(kMaxBlurCount * 2 + 1, 0.f);
	int limit = static_cast<int>(kMaxBlurCount);

	float _2_sigmaSqr = 2.f * sigma * sigma;
	float sum = 0.f;
	for (int i = -limit; i <= limit; ++i) {
		float x = static_cast<float>(i);
		float xSqr = x * x;
		float w = std::exp(-(xSqr) / _2_sigmaSqr);
		weights[i + kMaxBlurCount] = w;
		sum += w;
	}

	for (auto &w : weights)
		w /= sum;

	return weights;
}

std::size_t BlurFilter::getBlurRadiusBySigma(float sigma) {
	return std::min(static_cast<std::size_t>(std::ceil(2.f * sigma)), kMaxBlurRadius);
}

}
