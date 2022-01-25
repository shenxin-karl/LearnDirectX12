#include "Adapter.h"
#include "Device.h"
#include "DescriptorAllocator.h"
#include "DescriptorAllocation.h"
#include "DescriptorAllocatorPage.h"

using namespace dx12lib;

void testDescriptorAllocator(std::shared_ptr<Device> pDevice) {
	auto descriptor0 = pDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 5);
	auto descriptor1 = pDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 45);
	auto descriptor2 = pDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 46);
}

int main(int argc, char *argv[]) {
	std::shared_ptr<Adapter> pAdapter = std::make_shared<Adapter>();
	std::shared_ptr<Device> pDevice = std::make_shared<Device>(pAdapter);
	pDevice->initialize();
	testDescriptorAllocator(pDevice);
	return 0;
}
