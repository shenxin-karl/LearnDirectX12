#pragma once
#include "dx12libStd.h"
#include "DescriptorAllocation.h"
#include "IResource.h"
#include <DirectXColors.h>

namespace dx12lib {

enum class ClearFlag {
	None    = 0,
	Color   = (0x1 << 0),
	Depth   = (0x1 << 1),
	Stencil = (0x1 << 2),
};

ClearFlag operator|(ClearFlag lhs, ClearFlag rhs);
ClearFlag &operator|=(ClearFlag &lhs, ClearFlag rhs);
bool operator&(ClearFlag lhs, ClearFlag rhs);

class Texture : public Resource {
protected:
	Texture(std::weak_ptr<Device> pDevice, 
		const D3D12_RESOURCE_DESC &desc, 
		const D3D12_CLEAR_VALUE *pClearValue = nullptr
	);
	Texture(std::weak_ptr<Device> pDevice, 
		WRL::ComPtr<ID3D12Resource> pResource, 
		const D3D12_CLEAR_VALUE *pClearValue = nullptr
	);
	Texture(std::weak_ptr<Device> pDevice, 
		WRL::ComPtr<ID3D12Resource> pResource, 
		WRL::ComPtr<ID3D12Resource> pUploadHeap,
		D3D12_RESOURCE_STATES state
	);
public:
	void resize(uint32 width, uint32 height, uint32 depthOrArraySize = 1);
	D3D12_CPU_DESCRIPTOR_HANDLE getRenderTargetView() const;
	D3D12_CPU_DESCRIPTOR_HANDLE getDepthStencilView() const;
	D3D12_CPU_DESCRIPTOR_HANDLE getShaderResourceView() const;
	bool hasAlpha() const;
	size_t bitsPerPixel() const;
	bool checkRTVSupport() const noexcept;
	bool checkDSVSupport() const noexcept;
	bool checkSRVSupport() const noexcept;
	D3D12_RESOURCE_DESC getResourceDesc() const noexcept;
	uint32 getWidth() const noexcept;
	uint32 getHeight() const noexcept;
	uint32 getDepthOrArraySize() const noexcept;
	const D3D12_CLEAR_VALUE *getClearValue() const;
	D3D12_CLEAR_VALUE *getClearValue();
	void clearColor(DX::XMVECTORF32 color);
	void clearDepth(float depth);
	void clearStencil(UINT stencil);
	void setClearFlag(ClearFlag flag);
	void clearDepthStencil(float depth, UINT stencil);
	void clearColorDepthStencil(DX::XMVECTORF32 color, float depth, UINT stencil);
	ClearFlag getClearFlag() const;
private:
	void initializeClearValue(const D3D12_CLEAR_VALUE *pClearValue);
	void createViews();
private:
	DescriptorAllocation                _renderTargetView;
	DescriptorAllocation                _depthStencilView;
	DescriptorAllocation                _shaderResourceView;
	std::weak_ptr<Device>               _pDevice;
	D3D12_CLEAR_VALUE                   _clearValue;
	WRL::ComPtr<ID3D12Resource>         _pUploaderHeap;
	uint32    _width;
	uint32    _height;
	uint32    _depthOrArraySize;
	ClearFlag _clearFlag;
};

}