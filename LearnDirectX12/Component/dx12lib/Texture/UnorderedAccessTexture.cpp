#include <dx12lib/Texture/UnorderedAccessTexture.h>
#include <dx12lib/Device/Device.h>
#include <dx12lib/Tool/D3Dx12.h>
#include <dx12lib/Resource/ResourceStateTracker.h>

namespace dx12lib {

WRL::ComPtr<ID3D12Resource> UnorderedAccess2D::getD3DResource() const {
	return _pResource;
}

ShaderResourceView UnorderedAccess2D::getSRV(size_t mipSlice) const {
	if (_srvMgr.exist(mipSlice))
		return _srvMgr.get(mipSlice);

	assert(mipSlice < getMipmapLevels());
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
	ShaderResourceView SRV(descriptor, this);
	_srvMgr.set(mipSlice, SRV);
	return SRV;
}

UnorderedAccessView UnorderedAccess2D::getUAV(size_t mipSlice) const {
	if (_uavMgr.exist(mipSlice))
		return _uavMgr.get(mipSlice);

	assert(mipSlice < getMipmapLevels());
	auto pSharedDevice = _pDevice.lock();
	auto descriptor = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = _pResource->GetDesc().Format;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = static_cast<UINT>(mipSlice);
	uavDesc.Texture2D.PlaneSlice = 0;
	pSharedDevice->getD3DDevice()->CreateUnorderedAccessView(
		_pResource.Get(),
		nullptr,
		&uavDesc,
		descriptor.getCPUHandle()
	);
	UnorderedAccessView UAV(descriptor, this);
	_uavMgr.set(mipSlice, UAV);
	return UAV;
}

UnorderedAccess2D::UnorderedAccess2D(std::weak_ptr<Device> pDevice, WRL::ComPtr<ID3D12Resource> pResource,
	D3D12_RESOURCE_STATES state)
: _pDevice(pDevice), _pResource(pResource) {
	ResourceStateTracker::addGlobalResourceState(pResource.Get(), state);	
}

UnorderedAccess2D::UnorderedAccess2D(std::weak_ptr<Device> pDevice, size_t width, size_t height,
	const D3D12_CLEAR_VALUE *pClearValue, DXGI_FORMAT format)
: _pDevice(pDevice)
{
	auto pSharedDevice = pDevice.lock();
	if (format == DXGI_FORMAT_UNKNOWN)
		format = pSharedDevice->getDesc().backBufferFormat;

	D3D12_RESOURCE_DESC unorderedAccessDesc = {};
	unorderedAccessDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	unorderedAccessDesc.Alignment = 0;
	unorderedAccessDesc.Width = width;
	unorderedAccessDesc.Height = static_cast<UINT>(height);
	unorderedAccessDesc.DepthOrArraySize = 1;
	unorderedAccessDesc.Format = format;
	unorderedAccessDesc.MipLevels = 1;
	unorderedAccessDesc.SampleDesc.Count = 1;
	unorderedAccessDesc.SampleDesc.Quality = 0;
	unorderedAccessDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	unorderedAccessDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	ThrowIfFailed(pSharedDevice->getD3DDevice()->CreateCommittedResource(
		RVPtr(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT)),
		D3D12_HEAP_FLAG_NONE,
		RVPtr(unorderedAccessDesc),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		pClearValue,
		IID_PPV_ARGS(&_pResource)
	));
	ResourceStateTracker::addGlobalResourceState(_pResource.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

WRL::ComPtr<ID3D12Resource> UnorderedAccess2DArray::getD3DResource() const {
	return _pResource;
}

ShaderResourceView UnorderedAccess2DArray::getSRV(size_t mipSlice) const {
	if (_srvMgr.exist(mipSlice))
		return _srvMgr.get(mipSlice);

	auto pSharedDevice = _pDevice.lock();
	auto descriptor = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = _pResource->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Texture2DArray.MostDetailedMip = 0;
	srvDesc.Texture2DArray.MipLevels = -1;
	srvDesc.Texture2DArray.FirstArraySlice = 0;
	srvDesc.Texture2DArray.ArraySize = static_cast<UINT>(getPlaneSlice());
	srvDesc.Texture2DArray.PlaneSlice = 0;
	srvDesc.Texture2DArray.ResourceMinLODClamp = static_cast<float>(mipSlice);
	pSharedDevice->getD3DDevice()->CreateShaderResourceView(
		_pResource.Get(),
		&srvDesc,
		descriptor.getCPUHandle()
	);
	ShaderResourceView SRV(descriptor, this);
	_srvMgr.set(mipSlice, SRV);
	return SRV;
}

ShaderResourceView UnorderedAccess2DArray::getPlaneSRV(size_t planeSlice, size_t mipSlice) const {
	assert(planeSlice < getPlaneSlice());
	ViewManager<ShaderResourceView> &planeSrvMgr = _planeSrvMgr[planeSlice];
	if (planeSrvMgr.exist(mipSlice))
		return planeSrvMgr.get(mipSlice);

	auto pSharedDevice = _pDevice.lock();
	auto descriptor = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = _pResource->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
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
	ShaderResourceView SRV(descriptor, this);
	planeSrvMgr.set(mipSlice, SRV);
	return SRV;
}

UnorderedAccessView UnorderedAccess2DArray::getPlaneUAV(size_t planeSlice, size_t mipSlice) const {
	assert(planeSlice < getPlaneSlice());
	ViewManager<UnorderedAccessView> &planeUavMgr = _planeUavMgr[planeSlice];
	if (planeUavMgr.exist(mipSlice))
		return planeUavMgr.get(mipSlice);

	assert(mipSlice < getMipmapLevels());
	auto pSharedDevice = _pDevice.lock();
	auto descriptor = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = _pResource->GetDesc().Format;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2DArray.MipSlice = static_cast<UINT>(mipSlice);
	uavDesc.Texture2DArray.FirstArraySlice = static_cast<UINT>(planeSlice);
	uavDesc.Texture2DArray.ArraySize = 1;
	uavDesc.Texture2DArray.PlaneSlice = 0;
	pSharedDevice->getD3DDevice()->CreateUnorderedAccessView(
		_pResource.Get(),
		nullptr,
		&uavDesc,
		descriptor.getCPUHandle()
	);
	UnorderedAccessView UAV(descriptor, this);
	planeUavMgr.set(mipSlice, UAV);
	return UAV;
}

UnorderedAccess2DArray::UnorderedAccess2DArray(std::weak_ptr<Device> pDevice, WRL::ComPtr<ID3D12Resource> pResource,
	D3D12_RESOURCE_STATES state)
: _pDevice(pDevice), _pResource(pResource)
{
	ResourceStateTracker::addGlobalResourceState(pResource.Get(), state);
}

UnorderedAccess2DArray::UnorderedAccess2DArray(std::weak_ptr<Device> pDevice, size_t width, size_t height,
	size_t planeSlice, const D3D12_CLEAR_VALUE *pClearValue, DXGI_FORMAT format)
: _pDevice(pDevice)
{
	auto pSharedDevice = pDevice.lock();
	if (format == DXGI_FORMAT_UNKNOWN)
		format = pSharedDevice->getDesc().backBufferFormat;

	D3D12_RESOURCE_DESC unorderedAccessDesc = {};
	unorderedAccessDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	unorderedAccessDesc.Alignment = 0;
	unorderedAccessDesc.Width = width;
	unorderedAccessDesc.Height = static_cast<UINT>(height);
	unorderedAccessDesc.DepthOrArraySize = static_cast<UINT>(planeSlice);
	unorderedAccessDesc.MipLevels = 1;
	unorderedAccessDesc.Format = format;
	unorderedAccessDesc.SampleDesc.Count = 1;
	unorderedAccessDesc.SampleDesc.Quality = 0;
	unorderedAccessDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	unorderedAccessDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	ThrowIfFailed(pSharedDevice->getD3DDevice()->CreateCommittedResource(
		RVPtr(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT)),
		D3D12_HEAP_FLAG_NONE,
		RVPtr(unorderedAccessDesc),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		pClearValue,
		IID_PPV_ARGS(&_pResource)
	));
	ResourceStateTracker::addGlobalResourceState(_pResource.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

WRL::ComPtr<ID3D12Resource> UnorderedAccessCube::getD3DResource() const {
	return _pResource;
}

ShaderResourceView UnorderedAccessCube::getSRV(size_t mipSlice) const {
	if (_srvMgr.exist(mipSlice))
		return _srvMgr.get(mipSlice);

	auto pSharedDevice = _pDevice.lock();
	auto descriptor = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = _pResource->GetDesc().Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = -1;
	srvDesc.TextureCube.ResourceMinLODClamp = static_cast<float>(mipSlice);
	pSharedDevice->getD3DDevice()->CreateShaderResourceView(
		_pResource.Get(),
		&srvDesc,
		descriptor.getCPUHandle()
	);
	ShaderResourceView SRV(descriptor, this);
	_srvMgr.set(mipSlice, SRV);
	return SRV;
}

ShaderResourceView UnorderedAccessCube::getFaceSRV(CubeFace face, size_t mipSlice) const {
	ViewManager<ShaderResourceView> &cubeSrvMgr = _cubeSrvMgr[face];
	if (cubeSrvMgr.exist(mipSlice))
		return cubeSrvMgr.get(mipSlice);

	auto pSharedDevice = _pDevice.lock();
	auto descriptor = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = _pResource->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Texture2DArray.MostDetailedMip = 0;
	srvDesc.Texture2DArray.MipLevels = -1;
	srvDesc.Texture2DArray.FirstArraySlice = static_cast<UINT>(face);
	srvDesc.Texture2DArray.ArraySize = 1;
	srvDesc.Texture2DArray.PlaneSlice = 0;
	srvDesc.Texture2DArray.ResourceMinLODClamp = static_cast<float>(mipSlice);
	pSharedDevice->getD3DDevice()->CreateShaderResourceView(
		_pResource.Get(),
		&srvDesc,
		descriptor.getCPUHandle()
	);
	ShaderResourceView SRV(descriptor, this);
	cubeSrvMgr.set(mipSlice, SRV);
	return SRV;
}

UnorderedAccessView UnorderedAccessCube::getFaceUAV(CubeFace face, size_t mipSlice) const {
	ViewManager<UnorderedAccessView> &cubeUavMgr = _cubeUavMgr[face];
	if (cubeUavMgr.exist(mipSlice))
		return cubeUavMgr.get(mipSlice);

	assert(mipSlice < getMipmapLevels());
	auto pSharedDevice = _pDevice.lock();
	auto descriptor = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = _pResource->GetDesc().Format;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
	uavDesc.Texture2DArray.MipSlice = static_cast<UINT>(mipSlice);
	uavDesc.Texture2DArray.FirstArraySlice = static_cast<UINT>(face);
	uavDesc.Texture2DArray.ArraySize = 1;
	uavDesc.Texture2DArray.PlaneSlice = 0;
	pSharedDevice->getD3DDevice()->CreateUnorderedAccessView(
		_pResource.Get(),
		nullptr,
		&uavDesc,
		descriptor.getCPUHandle()
	);
	UnorderedAccessView UAV(descriptor, this);
	cubeUavMgr.set(mipSlice, UAV);
	return UAV;
}

UnorderedAccessView UnorderedAccessCube::get2DArrayUAV(size_t mipSlice) const {
	if (_2DArrayUavMgr.exist(mipSlice))
		return _2DArrayUavMgr.get(mipSlice);

	assert(mipSlice < getMipmapLevels());
	auto pSharedDevice = _pDevice.lock();
	auto descriptor = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = _pResource->GetDesc().Format;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
	uavDesc.Texture2DArray.MipSlice = static_cast<UINT>(mipSlice);
	uavDesc.Texture2DArray.FirstArraySlice = 0;
	uavDesc.Texture2DArray.ArraySize = 6;
	uavDesc.Texture2DArray.PlaneSlice = 0;
	pSharedDevice->getD3DDevice()->CreateUnorderedAccessView(
		_pResource.Get(),
		nullptr,
		&uavDesc,
		descriptor.getCPUHandle()
	);
	UnorderedAccessView UAV(descriptor, this);
	_2DArrayUavMgr.set(mipSlice, UAV);
	return UAV;
}

UnorderedAccessCube::UnorderedAccessCube(std::weak_ptr<Device> pDevice, WRL::ComPtr<ID3D12Resource> pResource,
                                         D3D12_RESOURCE_STATES state)
: _pDevice(pDevice), _pResource(pResource)
{
	ResourceStateTracker::addGlobalResourceState(pResource.Get(), state);
}

UnorderedAccessCube::UnorderedAccessCube(std::weak_ptr<Device> pDevice, size_t width, size_t height, size_t mipLevels,
	const D3D12_CLEAR_VALUE *pClearValue, DXGI_FORMAT format)
: _pDevice(pDevice)
{
	auto pSharedDevice = pDevice.lock();
	if (format == DXGI_FORMAT_UNKNOWN)
		format = pSharedDevice->getDesc().backBufferFormat;

	D3D12_RESOURCE_DESC unorderedAccessDesc = {};
	unorderedAccessDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	unorderedAccessDesc.Alignment = 0;
	unorderedAccessDesc.Width = width;
	unorderedAccessDesc.Height = static_cast<UINT>(height);
	unorderedAccessDesc.DepthOrArraySize = 6;
	unorderedAccessDesc.MipLevels = static_cast<UINT16>(mipLevels);
	unorderedAccessDesc.Format = format;
	unorderedAccessDesc.SampleDesc.Count = 1;
	unorderedAccessDesc.SampleDesc.Quality = 0;
	unorderedAccessDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	unorderedAccessDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	ThrowIfFailed(pSharedDevice->getD3DDevice()->CreateCommittedResource(
		RVPtr(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT)),
		D3D12_HEAP_FLAG_NONE,
		&unorderedAccessDesc,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		pClearValue,
		IID_PPV_ARGS(&_pResource)
	));
	ResourceStateTracker::addGlobalResourceState(_pResource.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
}


}
