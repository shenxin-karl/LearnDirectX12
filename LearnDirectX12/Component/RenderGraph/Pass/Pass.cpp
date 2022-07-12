#include "Pass.h"

namespace rg {

Pass::Pass(const std::string &passName) : _passName(passName) {
	_DSVFormat = DXGI_FORMAT_UNKNOWN;
	_blendDesc = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	_rasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	_depthStencilDesc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	_primitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
}

void Pass::setPassName(const std::string &passName) {
	_passName = passName;
}

const std::string & Pass::getPassName() const {
	return _passName;
}

const DXGI_FORMAT & Pass::getDSVFormat() const {
	return _DSVFormat;
}

const std::vector<DXGI_FORMAT> &Pass::getRTVFormats() const {
	return _RTVFormats;
}

const D3D12_BLEND_DESC & Pass::getBlendDesc() {
	return _blendDesc;
}

const D3D12_RASTERIZER_DESC & Pass::getRasterizerState() const {
	return _rasterizerState;
}

const D3D12_DEPTH_STENCIL_DESC & Pass::getDepthStencilDesc() const {
	return _depthStencilDesc;
}

const D3D12_PRIMITIVE_TOPOLOGY_TYPE & Pass::getPrimitiveTopologyType() const {
	return _primitiveTopologyType;
}

void Pass::setDSVFormat(const DXGI_FORMAT &DSVFormat) {
	_DSVFormat = DSVFormat;
}

void Pass::setRTVFormats(const std::vector<DXGI_FORMAT> &RTVFormats) {
	_RTVFormats = RTVFormats;
}

void Pass::setBlendDesc(const D3D12_BLEND_DESC &blendDesc) {
	_blendDesc = blendDesc;
}

void Pass::setRasterizerState(const D3D12_RASTERIZER_DESC &rasterizerDesc) {
	_rasterizerState = rasterizerDesc;
}

void Pass::setDepthStencilDesc(const D3D12_DEPTH_STENCIL_DESC &depthStencilDesc) {
	_depthStencilDesc = depthStencilDesc;
}

void Pass::setPrimitiveTopologyType(const D3D12_PRIMITIVE_TOPOLOGY_TYPE &primitiveTopologyType) {
	_primitiveTopologyType = primitiveTopologyType;
}

}
