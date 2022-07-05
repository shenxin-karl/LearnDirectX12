#include "StaticModelMaterial.h"

namespace d3d {

const std::string &StaticModelMaterial::getPSOName() const {
	static std::string t("aaa");
	return t;
}

void StaticModelMaterial::active(dx12lib::GraphicsContextProxy pGraphicsCtx) {

}

}
