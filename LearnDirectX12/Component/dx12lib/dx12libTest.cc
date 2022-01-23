#include "Adapter.h"
#include "Device.h"
#include "DescriptorAllocator.h"
#include "DescriptorAllocation.h"
#include "DescriptorAllocatorPage.h"

using namespace dx12lib;

void testDescriptorAllocator(std::weak_ptr<Device> pDevice) {
	DescriptorAllocator allocator(pDevice, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 50);
	auto descriptor0 = allocator.allocate(5);
	auto descriptor1 = allocator.allocate(45);
	auto descriptor2 = allocator.allocate(46);
}

int main(int argc, char *argv[]) {
	std::shared_ptr<Adapter> pAdapter = std::make_shared<Adapter>();
	std::shared_ptr<Device> pDevice = std::make_shared<Device>(pAdapter);
	testDescriptorAllocator(pDevice);
	return 0;
}
