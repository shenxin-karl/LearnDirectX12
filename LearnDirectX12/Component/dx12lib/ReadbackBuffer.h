#pragma once
#include "dx12libStd.h"
#include "IResource.h"


namespace dx12lib {

class ReadBackBuffer : public IResource {
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	ResourceType getResourceType() const override;
	~ReadBackBuffer() override;
	bool isCompleted() const;
	bool isMapped() const override;
	const void *getMapped() const;
protected:
	friend class CommandList;
	ReadBackBuffer(std::weak_ptr<Device> pDevice, std::size_t sizeInByte);
	void setCompleted(bool flag);
private:
	bool _isCompleted = false;
	mutable void *_pMapped = nullptr;
	std::size_t _bufferSize;
	WRL::ComPtr<ID3D12Resource> _pResource;
};

}