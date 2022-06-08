#include <D3D/Sky/SkyBox.h>
#include "Core/Utility/Utility.h"

namespace core {

void initDefaultSkyBox(dx12lib::GraphicsContextProxy pGraphicsCtx) {
	auto pSharedDevice = pGraphicsCtx->getDevice().lock();
	d3d::SkyBoxDesc desc = { pGraphicsCtx };
	desc.filename = L"Assets/SkyBox/DefaultSkyBox.dds";
	desc.renderTargetFormat = pSharedDevice->getDesc().backBufferFormat;
	desc.depthStencilFormat = pSharedDevice->getDesc().depthStencilFormat;
	pDefaultSkyBox = std::make_shared<d3d::SkyBox>(desc);
}

}
