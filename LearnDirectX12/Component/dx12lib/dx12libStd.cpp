#include "dx12libStd.h"

namespace dx12lib {

ResourceType operator|(const ResourceType &lhs, const ResourceType &rhs) {
	return ResourceType(std::size_t(lhs) | std::size_t(rhs));
}

bool operator&(const ResourceType &lhs, const ResourceType &rhs) {
	return std::size_t(lhs) & std::size_t(rhs);
}

}