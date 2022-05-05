#include "D3DShaderResource.h"

CMRC_DECLARE(D3D);

namespace d3d {

cmrc::file getD3DResource(const std::string &name) {
	auto fs = cmrc::D3D::get_filesystem();
	if (!fs.exists(name)) {
		assert(false);
		return {};
	}

	return fs.open(name);
}

}



