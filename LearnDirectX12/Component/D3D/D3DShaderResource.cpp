#include "D3DShaderResource.h"

CMRC_DECLARE(D3D);

namespace d3d {

cmrc::file getD3DShaderResource(const std::string &name) {
	auto fs = cmrc::D3D::get_filesystem();
	if (!fs.exists(name))
		return {};

	return fs.open(name);
}

}



