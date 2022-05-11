#pragma once
#include <dx12lib/dx12libStd.h>
#include <dx12lib/Resource/IResource.h>

namespace dx12lib {

class ReadBackBuffer : public IReadBackBuffer {
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	size_t getBufferSize() const override;
	bool isCompleted() const override;
	const void *getMappedPtr() const override;
	void setCompletedCallback(const std::function<void(IReadBackBuffer *)> &callback) override;
	~ReadBackBuffer() override;
protected:
	friend class CommandList;
	ReadBackBuffer(std::weak_ptr<Device> pDevice, std::size_t sizeInByte);
	void setCompleted(bool flag);
private:
	bool _isCompleted = false;
	mutable void *_pMapped = nullptr;
	std::size_t _bufferSize;
	WRL::ComPtr<ID3D12Resource> _pResource;
	std::function<void(IReadBackBuffer *)> _completedCallBack;
};

}
