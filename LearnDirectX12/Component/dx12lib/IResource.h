#pragma once
#include "dx12libStd.h"
#include "ResourceView.hpp"
#include <d3d12.h>
#include <wrl.h>

namespace dx12lib {

class IResource : public NonCopyable {
public:
	virtual WRL::ComPtr<ID3D12Resource> getD3DResource() const = 0;
	virtual ~IResource() = default;
	virtual uint64 getWidth() const;
	virtual uint64 getHeight() const;
	virtual uint64 getDepth() const;
	virtual bool isMapped() const;
	virtual DXGI_FORMAT getFormat() const;
	virtual size_t getMipmapLevels() const;
	virtual ResourceType getResourceType() const;
};

enum class ShaderResourceType {
	RenderTarget2D,
	RenderTarget3D,
	RenderTargetCube,
	Structured,
	UnorderedAccess,
	Texture2D,
	Texture3D,
	TextureCube,
	Texture2DArray,
};

enum class CubeFace : size_t {
	Right	= 0,
	Left	= 1,
	Top		= 2,
	Bottom	= 3,
	Back	= 4,
	Front	= 5,
	POSITIVE_X = 0,
	NEGATIVE_X = 1,
	POSITIVE_Y = 2,
	NEGATIVE_Y = 3,
	POSITIVE_Z = 4,
	NEGATIVE_Z = 5,
};

class IShaderResourceBuffer : virtual public IResource {
public:
	virtual ShaderResourceView getShaderResourceView(size_t mipLevel = 0) const = 0;
	virtual ShaderResourceType getShaderResourceType() const = 0;
};

class IRenderTargetBuffer : virtual public IResource {
public:
	virtual RenderTargetView getRenderTargetView(size_t mipSlice = 0) const = 0;
};

class IDepthStencilBuffer : virtual public IResource {
public:
	virtual DepthStencilView getDepthStencilView() const = 0;
	virtual ShaderResourceView getShaderResourceView() const = 0;
};

class IUnorderedAccessBuffer : virtual public IResource {
public:
	virtual UnorderedAccessView getUnorderedAccessView(size_t mipSlice = 0) const = 0;
};


class IConstantBuffer : virtual public IResource {
public:
 	virtual ConstantBufferView getConstantBufferView() const = 0;
	virtual void updateConstantBuffer(const void *pData, size_t sizeInByte, size_t offset = 0) = 0;
	virtual size_t getConstantBufferSize() const noexcept = 0;
};

class IStructuredBuffer : virtual public IResource {
public:
	bool isMapped() const override { return true; }
	virtual ShaderResourceView getShaderResourceView() const = 0;
	virtual void updateStructuredBuffer(const void *pData, size_t sizeInByte, size_t offset = 0) = 0;
	virtual size_t getStructuredBufferSize() const = 0;
	virtual size_t getElementCount() const = 0;
	virtual size_t getStride() const = 0;
};

}