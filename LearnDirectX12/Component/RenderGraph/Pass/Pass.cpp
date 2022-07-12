#include "Pass.h"

namespace rg {

Pass::Pass(const std::string &passName) : _passName(passName) {
}

void Pass::setPassName(const std::string &passName) {
	_passName = passName;
}

const std::string & Pass::getPassName() const {
	return _passName;
}

}
