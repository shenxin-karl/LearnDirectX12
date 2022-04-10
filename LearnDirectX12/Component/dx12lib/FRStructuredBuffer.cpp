#include "FRStructuredBuffer.hpp"

namespace dx12lib {

FRStructuredBuffer<RawData>::FRStructuredBuffer(std::weak_ptr<Device> pDevice, const void *pData, size_t numElements, size_t stride) {
	size_t sizeInByte = numElements * stride;
	auto pSharedDevice = pDevice.lock();
	_structuredBufferView = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	_pUploadBuffer = std::make_unique<UploadBuffer>(
		pSharedDevice->getD3DDevice(),
		kFrameResourceCount,
		sizeInByte,
		false
	);

	D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
	desc.Format = _pUploadBuffer->getD3DResource()->GetDesc().Format;
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	desc.Buffer.NumElements = static_cast<UINT>(numElements);
	desc.Buffer.StructureByteStride = static_cast<UINT>(stride);
	desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	_structuredBufferView = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kFrameResourceCount);
	for (size_t i = 0; i < kFrameResourceCount; ++i) {
		if (pData != nullptr)
			_pUploadBuffer->copyData(i, pData, sizeInByte, 0);

		desc.Buffer.FirstElement = i * numElements;
		pSharedDevice->getD3DDevice()->CreateShaderResourceView(
			_pUploadBuffer->getD3DResource().Get(),
			&desc,
			_structuredBufferView.getCPUHandle(i)
		);
	}
}

WRL::ComPtr<ID3D12Resource> FRStructuredBuffer<RawData>::getD3DResource() const {
	return _pUploadBuffer->getD3DResource();
}

D3D12_CPU_DESCRIPTOR_HANDLE FRStructuredBuffer<RawData>::getShaderResourceView() const {
	size_t frameIndex = FrameIndexProxy::getConstantFrameIndexRef();
	return _structuredBufferView.getCPUHandle(frameIndex);
}

void FRStructuredBuffer<RawData>::updateStructuredBuffer(const void *pData, size_t sizeInByte, size_t offset) {
	size_t frameIndex = FrameIndexProxy::getConstantFrameIndexRef();
	_pUploadBuffer->copyData(frameIndex, pData, sizeInByte, offset);
}

size_t FRStructuredBuffer<RawData>::getStructuredBufferSize() const {
	return _pUploadBuffer->getWidth();
}

size_t FRStructuredBuffer<RawData>::getElementCount() const {
	return _pUploadBuffer->getElementCount();
}

size_t FRStructuredBuffer<RawData>::getStride() const {
	return _pUploadBuffer->getElementByteSize();
}

}
