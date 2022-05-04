#include <dx12lib/Texture/RenderTargetTexture.h>
#include <dx12lib/Device/Device.h>
#include <dx12lib/Resource/ResourceStateTracker.h>

namespace dx12lib {

WRL::ComPtr<ID3D12Resource> RenderTarget2D::getD3DResource() const {
	return _pResource;
}

ShaderResourceView RenderTarget2D::getSRV(size_t mipSlice) const {
	assert(mipSlice < _pResource->GetDesc().MipLevels);
	if (_srvMgr.exist(mipSlice))
		return _srvMgr.get(mipSlice);

	auto pSharedDevice = _pDevice.lock();
	auto descriptor = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = _pResource->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;
	srvDesc.Texture2D.PlaneSlice = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = static_cast<float>(mipSlice);

	pSharedDevice->getD3DDevice()->CreateShaderResourceView(
		_pResource.Get(),
		&srvDesc,
		descriptor.getCPUHandle()
	);

	ShaderResourceView SRV(descriptor);
	_srvMgr.set(mipSlice, SRV);
	return SRV;
}

RenderTargetView RenderTarget2D::getRTV(size_t mipSlice) const {
	if (_rtvMgr.exist(mipSlice))
		return _rtvMgr.get(mipSlice);

	assert(mipSlice < _pResource->GetDesc().MipLevels);
	auto pSharedDevice = _pDevice.lock();
	auto descriptor = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = _pResource->GetDesc().Format;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.PlaneSlice = 0;
	rtvDesc.Texture2D.MipSlice = static_cast<UINT>(mipSlice);

	pSharedDevice->getD3DDevice()->CreateRenderTargetView(
		_pResource.Get(),
		&rtvDesc,
		descriptor.getCPUHandle()
	);

	RenderTargetView RTV(descriptor);
	_rtvMgr.set(mipSlice, RTV);
	return RTV;
}

RenderTarget2D::RenderTarget2D(std::weak_ptr<Device> 
                                       pDevice, WRL::ComPtr<ID3D12Resource> pResource,
                                       D3D12_RESOURCE_STATES state)
: _pDevice(pDevice), _pResource(pResource)
{
	ResourceStateTracker::addGlobalResourceState(_pResource.Get(), state);
}

RenderTarget2D::RenderTarget2D(std::weak_ptr<Device> pDevice, size_t width, size_t height,
	const D3D12_CLEAR_VALUE *pClearValue, DXGI_FORMAT format)
: _pDevice(pDevice)
{

	auto pSharedDevice = pDevice.lock();
	if (format == DXGI_FORMAT_UNKNOWN)
		format = pSharedDevice->getDesc().backBufferFormat;

	D3D12_RESOURCE_DESC renderTargetDesc;
	renderTargetDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	renderTargetDesc.Alignment = 0;
	renderTargetDesc.Width = width;
	renderTargetDesc.Height = static_cast<UINT>(height);
	renderTargetDesc.DepthOrArraySize = 1;
	renderTargetDesc.MipLevels = 1;
	renderTargetDesc.Format = format;
	renderTargetDesc.SampleDesc.Count = 1;
	renderTargetDesc.SampleDesc.Quality = 0;
	renderTargetDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	renderTargetDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	ThrowIfFailed(pSharedDevice->getD3DDevice()->CreateCommittedResource(
		RVPtr(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT)),
		D3D12_HEAP_FLAG_NONE,
		RVPtr(renderTargetDesc),
		D3D12_RESOURCE_STATE_COMMON,
		pClearValue,
		IID_PPV_ARGS(&_pResource)
	));

	ResourceStateTracker::addGlobalResourceState(_pResource.Get(), D3D12_RESOURCE_STATE_COMMON);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////

WRL::ComPtr<ID3D12Resource> RenderTarget2DArray::getD3DResource() const {
	return _pResource;
}

ShaderResourceView RenderTarget2DArray::getSRV(size_t mipSlice) const {
	if (_srvMgr.exist(mipSlice))
		return _srvMgr.get(mipSlice);

	auto pSharedDevice = _pDevice.lock();
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = _pResource->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2DArray.MostDetailedMip = 0;
	srvDesc.Texture2DArray.MipLevels = -1;
	srvDesc.Texture2DArray.FirstArraySlice = 0;
	srvDesc.Texture2DArray.ArraySize = _pResource->GetDesc().DepthOrArraySize;
	srvDesc.Texture2DArray.PlaneSlice = 0;
	srvDesc.Texture2DArray.ResourceMinLODClamp = static_cast<float>(mipSlice);
	auto descriptor = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	pSharedDevice->getD3DDevice()->CreateShaderResourceView(
		_pResource.Get(),
		&srvDesc,
		descriptor.getCPUHandle()
	);

	ShaderResourceView SRV(descriptor);
	_srvMgr.set(mipSlice, SRV);
	return SRV;
}

ShaderResourceView RenderTarget2DArray::getSRV(size_t planeSlice, size_t mipSlice) const {
	assert(planeSlice < getPlaneSlice());
	ViewManager<ShaderResourceView> &planeSrvMgr = _planeSrvMgr[planeSlice];
	if (planeSrvMgr.exist(mipSlice))
		return planeSrvMgr.get(mipSlice);

	auto pSharedDevice = _pDevice.lock();
	auto descriptor = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = _pResource->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2DArray.MostDetailedMip = 0;
	srvDesc.Texture2DArray.MipLevels = -1;
	srvDesc.Texture2DArray.FirstArraySlice = static_cast<UINT>(planeSlice);
	srvDesc.Texture2DArray.ArraySize = 1;
	srvDesc.Texture2DArray.PlaneSlice = 0;
	srvDesc.Texture2DArray.ResourceMinLODClamp = static_cast<float>(mipSlice);
	pSharedDevice->getD3DDevice()->CreateShaderResourceView(
		_pResource.Get(),
		&srvDesc,
		descriptor.getCPUHandle()
	);

	ShaderResourceView SRV(descriptor);
	planeSrvMgr.set(mipSlice, SRV);
	return SRV;
}

RenderTargetView RenderTarget2DArray::getRTV(size_t planeSlice, size_t mipSlice) const {
	assert(planeSlice < getPlaneSlice());
	ViewManager<RenderTargetView> &planeRtvMgr = _planeRtvMgr[planeSlice];
	if (planeRtvMgr.exist(mipSlice))
		return planeRtvMgr.get(mipSlice);

	auto pSharedDevice = _pDevice.lock();
	auto descriptor = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = _pResource->GetDesc().Format;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
	rtvDesc.Texture2DArray.MipSlice = static_cast<UINT>(mipSlice);
	rtvDesc.Texture2DArray.FirstArraySlice = static_cast<UINT>(planeSlice);
	rtvDesc.Texture2DArray.ArraySize = 1;
	rtvDesc.Texture2DArray.PlaneSlice = 0;
	pSharedDevice->getD3DDevice()->CreateRenderTargetView(
		_pResource.Get(),
		&rtvDesc,
		descriptor.getCPUHandle()
	);

	RenderTargetView RTV(descriptor);
	planeRtvMgr.set(mipSlice, RTV);
	return RTV;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

RenderTarget2DArray::RenderTarget2DArray(std::weak_ptr<Device> pDevice, 
	WRL::ComPtr<ID3D12Resource> pResource,
	D3D12_RESOURCE_STATES state)
: _pDevice(pDevice), _pResource(pResource)
{
	ResourceStateTracker::addGlobalResourceState(pResource.Get(), state);
}

RenderTarget2DArray::RenderTarget2DArray(std::weak_ptr<Device> pDevice, 
	size_t width, 
	size_t height, 
	size_t planeSlice,
	const D3D12_CLEAR_VALUE *pClearValue, 
	DXGI_FORMAT format)
: _pDevice(pDevice)
{
	assert(planeSlice >= 1);
	auto pSharedDevice = pDevice.lock();
	if (format == DXGI_FORMAT_UNKNOWN)
		format = pSharedDevice->getDesc().backBufferFormat;

	D3D12_RESOURCE_DESC renderTargetDesc;
	renderTargetDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	renderTargetDesc.Alignment = 0;
	renderTargetDesc.Width = width;
	renderTargetDesc.Height = static_cast<UINT>(height);
	renderTargetDesc.DepthOrArraySize = static_cast<UINT16>(planeSlice);
	renderTargetDesc.MipLevels = 1;
	renderTargetDesc.Format = format;
	renderTargetDesc.SampleDesc.Count = 1;
	renderTargetDesc.SampleDesc.Quality = 0;
	renderTargetDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	renderTargetDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	ThrowIfFailed(pSharedDevice->getD3DDevice()->CreateCommittedResource(
		RVPtr(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT)),
		D3D12_HEAP_FLAG_NONE,
		RVPtr(renderTargetDesc),
		D3D12_RESOURCE_STATE_COMMON,
		pClearValue,
		IID_PPV_ARGS(&_pResource)
	));

	ResourceStateTracker::addGlobalResourceState(_pResource.Get(), D3D12_RESOURCE_STATE_COMMON);
}

WRL::ComPtr<ID3D12Resource> RenderTargetCube::getD3DResource() const {
	return _pResource;
}

ShaderResourceView RenderTargetCube::getSRV(size_t mipSlice) const {
	if (_srvMgr.exist(mipSlice))
		return _srvMgr.get(mipSlice);

	assert(mipSlice < getMipmapLevels());
	auto pSharedDevice = _pDevice.lock();
	auto descriptor = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = _pResource->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = -1;
	srvDesc.TextureCube.ResourceMinLODClamp = static_cast<float>(mipSlice);
	pSharedDevice->getD3DDevice()->CreateShaderResourceView(
		_pResource.Get(),
		&srvDesc,
		descriptor.getCPUHandle()
	);

	ShaderResourceView SRV(descriptor);
	_srvMgr.set(mipSlice, SRV);
	return SRV;
}

RenderTargetView RenderTargetCube::getRTV(CubeFace face, size_t mipSlice) const {
	ViewManager<RenderTargetView> &cubeRtvMgr = _cubeRtvMgr[face];
	if (cubeRtvMgr.exist(mipSlice))
		return cubeRtvMgr.get(mipSlice);

	auto pSharedDevice = _pDevice.lock();
	auto descriptor = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = _pResource->GetDesc().Format;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
	rtvDesc.Texture2DArray.MipSlice = static_cast<UINT>(mipSlice);
	rtvDesc.Texture2DArray.FirstArraySlice = static_cast<UINT>(face);
	rtvDesc.Texture2DArray.ArraySize = 1;
	rtvDesc.Texture2DArray.PlaneSlice = 0;
	pSharedDevice->getD3DDevice()->CreateRenderTargetView(
		_pResource.Get(),
		&rtvDesc,
		descriptor.getCPUHandle()
	);

	RenderTargetView RTV(descriptor);
	cubeRtvMgr.set(mipSlice, RTV);
	return RTV;
}

RenderTargetCube::RenderTargetCube(std::weak_ptr<Device> pDevice,
	WRL::ComPtr<ID3D12Resource> pResource,
	D3D12_RESOURCE_STATES state)
: _pDevice(pDevice), _pResource(pResource)
{
	ResourceStateTracker::addGlobalResourceState(pResource.Get(), state);
}

RenderTargetCube::RenderTargetCube(std::weak_ptr<Device> pDevice, uint32 width, uint32 height,
	D3D12_CLEAR_VALUE *pClearValue, DXGI_FORMAT format)
: _pDevice(pDevice)
{
	auto pSharedDevice = pDevice.lock();
	if (format == DXGI_FORMAT_UNKNOWN)
		format = pSharedDevice->getDesc().backBufferFormat;

	D3D12_RESOURCE_DESC renderTargetDesc;
	renderTargetDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	renderTargetDesc.Alignment = 0;
	renderTargetDesc.Width = width;
	renderTargetDesc.Height = height;
	renderTargetDesc.DepthOrArraySize = 6;
	renderTargetDesc.MipLevels = 1;
	renderTargetDesc.Format = format;
	renderTargetDesc.SampleDesc.Count = 1;
	renderTargetDesc.SampleDesc.Quality = 0;
	renderTargetDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	renderTargetDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	ThrowIfFailed(pSharedDevice->getD3DDevice()->CreateCommittedResource(
		RVPtr(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT)),
		D3D12_HEAP_FLAG_NONE,
		RVPtr(renderTargetDesc),
		D3D12_RESOURCE_STATE_COMMON,
		pClearValue,
		IID_PPV_ARGS(&_pResource)
	));
	ResourceStateTracker::addGlobalResourceState(_pResource.Get(), D3D12_RESOURCE_STATE_COMMON);
}

}
