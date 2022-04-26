#include "Device.h"
#include "IResource.h"
#include "ResourceStateTracker.h"

namespace dx12lib {

uint64 IResource::getWidth() const {
	return getD3DResource()->GetDesc().Width;
}

uint64 IResource::getHeight() const {
	return getD3DResource()->GetDesc().Height;
}

uint64 IResource::getDepth() const {
	return getD3DResource()->GetDesc().DepthOrArraySize;
}

bool IResource::isMapped() const {
	return false;
}

DXGI_FORMAT IResource::getFormat() const {
	return getD3DResource()->GetDesc().Format;
}

size_t IResource::getMipmapLevels() const {
	return getD3DResource()->GetDesc().MipLevels;
}

}
