#include "BlurFilter.h"
#include "D3Dx12.h"
#include "dx12lib/IResource.h"
#include "dx12lib/PipelineStateObject.h"
#include "dx12lib/RootSignature.h"
#include "dx12lib/Device.h"
#include "D3DShaderResource.h"
#include "d3dutil.h" 
#include "dx12lib/UnorderedAccessBuffer.h"
#include "dx12lib/ResourceStateTracker.h"


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
	buildUnorderedAccessResouce(pComputeContext);
	buildBlurPSO(pComputeContext->getDevice());
}

void BlurFilter::onResize(dx12lib::ComputeContextProxy pComputeList, std::uint32_t width, std::uint32_t height) {
	if (_width != width || _height != height) {
		_width = width;
		_height = height;
		buildUnorderedAccessResouce(pComputeList);
	}
}

void BlurFilter::produceImpl(dx12lib::ComputeContextProxy pComputeList,
	std::shared_ptr<dx12lib::IShaderSourceResource> pShaderResource,
	int blurCount,
	float sigma)
{
	auto setConstantBuffer = [&, weights = calcGaussianWeights(blurCount, sigma)]() {
		pComputeList->setCompute32BitConstants(CB_BlurParame, 1, &blurCount);
		pComputeList->setCompute32BitConstants(CB_BlurParame, 11, weights.data(), 1);
	};

	assert(pShaderResource->isShaderSample());
	//updateBlurConstantBuffer(pComputeList, blurCount, sigma);
	pComputeList->copyResource(_pBlurMap1, pShaderResource);
	for (int i = 0; i < blurCount; ++i) {
		// horizonal blur
		pComputeList->setPipelineStateObject(_pHorzBlurPSO);
		pComputeList->setShaderResourceBuffer(_pBlurMap1, SR_Input);
		pComputeList->setUnorderedAccessBuffer(_pBlurMap0, UA_Output);
		setConstantBuffer();
		pComputeList->transitionBarrier(_pBlurMap1, D3D12_RESOURCE_STATE_GENERIC_READ);
		pComputeList->transitionBarrier(_pBlurMap0, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		int numXGroup = static_cast<int>(std::ceil(_width / float(kMaxThreads)));
		pComputeList->dispatch(numXGroup, _height, 1);
		
		// vertical blur
		pComputeList->setPipelineStateObject(_pVertBlurPSO);
		pComputeList->setShaderResourceBuffer(_pBlurMap0, SR_Input);
		pComputeList->setUnorderedAccessBuffer(_pBlurMap1, UA_Output);
		setConstantBuffer();
		pComputeList->transitionBarrier(_pBlurMap0, D3D12_RESOURCE_STATE_GENERIC_READ);
		pComputeList->transitionBarrier(_pBlurMap1, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		int numYGroup = static_cast<int>(std::ceil(_height / float(kMaxThreads)));
		pComputeList->dispatch(_width, numYGroup, 1);
	}
}

std::shared_ptr<dx12lib::UnorderedAccessBuffer> BlurFilter::getOuput() const {
	return _pBlurMap1;
}

void BlurFilter::buildUnorderedAccessResouce(dx12lib::ComputeContextProxy pComputeContext) {
	_pBlurMap0 = pComputeContext->createUnorderedAccessBuffer(_width, _height, _format);
	_pBlurMap1 = pComputeContext->createUnorderedAccessBuffer(_width, _height, _format);
}

void BlurFilter::buildBlurPSO(std::weak_ptr<dx12lib::Device> pDevice) {
	if (_pHorzBlurPSO == nullptr) {
		auto pBlurFilterCSFile = getD3DShaderResource("shader/BlurFilterCS.hlsl");
		assert(pBlurFilterCSFile.size() != 0);

		dx12lib::RootSignatureDescHelper desc;
		desc.resize(3);
		desc[SR_Input].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);;
		desc[UA_Output].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
		desc[CB_BlurParame].InitAsConstants(12, 0);
		auto pSharedDevice = pDevice.lock();
		auto pRootSignature = pSharedDevice->createRootSignature(desc);

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
	pComputeList->setCompute32BitConstants(CB_BlurParame, 1, &blurCount);
	pComputeList->setCompute32BitConstants(CB_BlurParame, 11, weights.data(), 1);
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

std::size_t BlurFilter::getBlorRadiusBySigma(float sigma) {
	return static_cast<std::size_t>(std::ceil(2.f * sigma));
}

}