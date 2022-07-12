#pragma once
#include <string>
#include <RenderGraph/RenderGraphStd.h>
#include <dx12lib/Context/ContextProxy.hpp>

namespace rg {

class Pass : public NonCopyable {
public:
	explicit Pass(const std::string &passName);
	virtual void execute(dx12lib::GraphicsContextProxy pGraphicsCtx) const = 0;
	virtual void reset() {}
	void setPassName(const std::string &passName);
	const std::string &getPassName() const;
	const DXGI_FORMAT &getDSVFormat() const;
	const std::vector<DXGI_FORMAT> &getRTVFormats() const;
	const D3D12_BLEND_DESC &getBlendDesc();
	const D3D12_RASTERIZER_DESC &getRasterizerState() const;
	const D3D12_DEPTH_STENCIL_DESC &getDepthStencilDesc() const;
	const D3D12_PRIMITIVE_TOPOLOGY_TYPE &getPrimitiveTopologyType() const;
	void setDSVFormat(const DXGI_FORMAT &DSVFormat);
	void setRTVFormats(const std::vector<DXGI_FORMAT> &RTVFormats);
	void setBlendDesc(const D3D12_BLEND_DESC &blendDesc);
	void setRasterizerState(const D3D12_RASTERIZER_DESC &rasterizerDesc);
	void setDepthStencilDesc(const D3D12_DEPTH_STENCIL_DESC &depthStencilDesc);
	void setPrimitiveTopologyType(const D3D12_PRIMITIVE_TOPOLOGY_TYPE &primitiveTopologyType);
private:
	std::string _passName;
	DXGI_FORMAT _DSVFormat;
	std::vector<DXGI_FORMAT> _RTVFormats;
	D3D12_BLEND_DESC _blendDesc;
	D3D12_RASTERIZER_DESC _rasterizerState;
	D3D12_DEPTH_STENCIL_DESC _depthStencilDesc;
	D3D12_PRIMITIVE_TOPOLOGY_TYPE _primitiveTopologyType;
};

}
