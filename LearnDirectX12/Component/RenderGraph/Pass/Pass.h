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
