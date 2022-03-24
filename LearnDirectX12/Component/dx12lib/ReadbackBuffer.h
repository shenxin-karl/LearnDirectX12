#pragma once
#include "dx12libStd.h"
#include "IResource.h"


namespace dx12lib {

class ReadbackBuffer : public IResource {
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	~ReadbackBuffer() override;
	bool isCompleted() const;
	bool isMapped() const override;
	const void *getMapped() const;
protected:
	friend class CommandList;
	ReadbackBuffer(std::weak_ptr<Device> pDevice, std::size_t sizeInByte);
	void setCompleted(bool flag);
private:
	bool _isCompleted = false;
	mutable void *_pMapped = nullptr;
	std::size_t _bufferSize;
	WRL::ComPtr<ID3D12Resource> _pResource;
};

}