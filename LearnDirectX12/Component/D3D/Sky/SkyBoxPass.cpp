#include "SkyBoxPass.h"

namespace d3d {

SkyBoxPass::SkyBoxPass(const std::string &passName) : GraphicsPass(passName, true, true)  {
}

void SkyBoxPass::execute(dx12lib::DirectContextProxy pDirectCtx) {
	GraphicsPass::execute(pDirectCtx);
}

}
