#include <dx12lib/Buffer/IndexBuffer.h>
#include <dx12lib/Buffer/DefaultBuffer.h>
#include <dx12lib/Device/Device.h>
#include <dx12lib/Context/CommandList.h>

namespace dx12lib {

IndexBuffer::IndexBuffer(std::weak_ptr<Device> pDevice,
	std::shared_ptr<CommandList> pCmdList,
	const void *pData,
	size_t numElements,
	DXGI_FORMAT format)
: _indexFormat(format)
{
	auto stride = getIndexStrideByFormat(format);
	size_t sizeInByte = numElements * stride;
	assert(stride != 0 && "invalid index type");
	_pDefaultBuffer = std::make_unique<DefaultBuffer>(
		pDevice.lock()->getD3DDevice(), 
		pCmdList->getD3DCommandList(), 
		pData, 
		sizeInByte
	);
}
WRL::ComPtr<ID3D12Resource> IndexBuffer::getD3DResource() const {
	return _pDefaultBuffer->getD3DResource();
}

size_t IndexBuffer::getBufferSize() const {
	return _pDefaultBuffer->getBufferSize();
}

size_t IndexBuffer::getIndexCount() const {
	return getBufferSize() / getIndexStrideByFormat(_indexFormat);
}

size_t IndexBuffer::getIndexStride() const {
	return getIndexStrideByFormat(_indexFormat);
}

DXGI_FORMAT IndexBuffer::getIndexFormat() const {
	return _indexFormat;
}

IndexBufferView IndexBuffer::getIndexBufferView() const {
	return IndexBufferView(_pDefaultBuffer->getAddress(), static_cast<UINT>(getBufferSize()), _indexFormat);
}

size_t IndexBuffer::getIndexStrideByFormat(DXGI_FORMAT format) {
	switch (format) {
	case DXGI_FORMAT_R8_SINT:
	case DXGI_FORMAT_R8_UINT:
		return 1;
	case DXGI_FORMAT_R16_SINT:
	case DXGI_FORMAT_R16_UINT:
		return 2;
	case DXGI_FORMAT_R32_SINT:
	case DXGI_FORMAT_R32_UINT:
		return 4;
	default:
		assert(false);
		return 0;
	}
}

}