#pragma once
#include <dx12lib/dx12libStd.h>
#include <dx12lib/Resource/ResourceView.hpp>
#include <d3d12.h>
#include <wrl.h>

namespace dx12lib {

interface IResource : NonCopyable {
	~IResource() override = default;
	void setResourceName(const std::string &name);
	void setResourceName(const std::wstring &name);
	virtual WRL::ComPtr<ID3D12Resource> getD3DResource() const = 0;
};

interface IShaderResource : virtual IResource {
	virtual ShaderResourceDimension getDimension() const = 0;
	virtual ShaderResourceView getSRV(size_t mipSlice = 0) const = 0;
	virtual bool checkSRVState(D3D12_RESOURCE_STATES currentState) const;
	virtual size_t getWidth() const;
	virtual size_t getHeight() const;
	virtual size_t getMipmapLevels() const;
	virtual DXGI_FORMAT getFormat() const;
};

/////////////////////////////////////////////ShaderResource/////////////////////////////////////////////
interface IShaderResource2D : IShaderResource {
	ShaderResourceDimension getDimension() const override;
};

interface IShaderResource2DArray : IShaderResource {
	ShaderResourceDimension getDimension() const override;
	virtual size_t getPlaneSlice() const;
	virtual ShaderResourceView getPlaneSRV(size_t planeSlice, size_t mipSlice = 0) const = 0;
};

interface IShaderResourceCube : IShaderResource {
	ShaderResourceDimension getDimension() const override;
	virtual ShaderResourceView getFaceSRV(CubeFace face, size_t mipSlice = 0) const = 0;
};
/////////////////////////////////////////////RenderTarget/////////////////////////////////////////////
interface IRenderTarget : virtual IResource {
	virtual bool checkRTVState(D3D12_RESOURCE_STATES currentState) const;
};

interface IRenderTarget2D : IRenderTarget, IShaderResource2D {
	virtual RenderTargetView getRTV(size_t mipSlice = 0) const = 0;
};

interface IRenderTarget2DArray : IRenderTarget, IShaderResource2DArray {
	virtual RenderTargetView getPlaneRTV(size_t planeSlice, size_t mipSlice = 0) const = 0;
};

interface IRenderTargetCube : IRenderTarget, IShaderResourceCube {
	virtual RenderTargetView getFaceRTV(CubeFace face, size_t mipSlice = 0) const = 0;
};
/////////////////////////////////////////////UnorderedAccess/////////////////////////////////////////////
interface IUnorderedAccess : virtual IResource {
	virtual bool checkUAVState(D3D12_RESOURCE_STATES currentState) const;
};

interface IUnorderedAccess2D : IUnorderedAccess, IShaderResource2D {
	virtual UnorderedAccessView getUAV(size_t mipSlice = 0) const = 0;
};

interface IUnorderedAccess2DArray : IUnorderedAccess, IShaderResource2DArray {
	virtual UnorderedAccessView getPlaneUAV(size_t planeSlice, size_t mipSlice = 0) const = 0;
};

interface IUnorderedAccessCube : IUnorderedAccess, IShaderResourceCube {
	virtual UnorderedAccessView getFaceUAV(CubeFace face, size_t mipSlice = 0) const = 0;
};
/////////////////////////////////////////////IDepthStencil2D/////////////////////////////////////////////
interface IDepthStencil : virtual IResource {
	virtual bool checkDSVState(D3D12_RESOURCE_STATES currentState) const;
};

interface IDepthStencil2D : IDepthStencil, IShaderResource2D {
	bool checkSRVState(D3D12_RESOURCE_STATES currentState) const override;
	virtual DepthStencilView getDSV() const = 0;
};

/////////////////////////////////////////////IBufferResource/////////////////////////////////////////////

interface IBufferResource : IResource {
	virtual BufferType getBufferType() const = 0;
	virtual size_t getBufferSize() const = 0;
};
/////////////////////////////////////////////IConstantBuffer/////////////////////////////////////////////
interface IConstantBuffer : IBufferResource {
	BufferType getBufferType() const override;
	virtual size_t getElementStride() const = 0;
	virtual BYTE *getMappedPtr() = 0;
	virtual const BYTE *getMappedPtr() const = 0;
	virtual void updateBuffer(const void *pData, size_t sizeInByte, size_t offset = 0) = 0;
	virtual ConstantBufferView getCBV() const = 0;
	virtual bool checkCBVState(D3D12_RESOURCE_STATES currentState) const;
};
/////////////////////////////////////////////IVertexBuffer/////////////////////////////////////////////
interface IVertexBuffer : IBufferResource {
	BufferType getBufferType() const override;
	virtual size_t getVertexCount() const = 0;
	virtual size_t getVertexStride() const = 0;
	virtual VertexBufferView getVertexBufferView() const = 0;
};
/////////////////////////////////////////////IIndexBuffer/////////////////////////////////////////////
interface IIndexBuffer : IBufferResource {
	BufferType getBufferType() const override;
	virtual size_t getIndexCount() const = 0;
	virtual size_t getIndexStride() const = 0;
	virtual DXGI_FORMAT getIndexFormat() const = 0;
	virtual IndexBufferView getIndexBufferView() const = 0;
};
//////////////////////////////////////////IStructuredBuffer//////////////////////////////////////////
interface IStructuredBuffer : IBufferResource {
	BufferType getBufferType() const override;
	bool checkSRVState(D3D12_RESOURCE_STATES state) const;
	virtual size_t getElementCount() const = 0;
	virtual size_t getElementStride() const = 0;
	virtual BYTE *getMappedPtr() = 0;
	virtual const BYTE *getMappedPtr() const = 0;
	virtual void updateBuffer(const void *pData, size_t sizeInByte, size_t offset = 0) = 0;
	virtual StructuredBufferView getSRV() const = 0;
};
//////////////////////////////////////////IReadBackBuffer//////////////////////////////////////////
interface IReadBackBuffer : IBufferResource {
	BufferType getBufferType() const override;
	virtual bool isCompleted() const = 0;
	virtual const void *getMappedPtr() const = 0;
};

}