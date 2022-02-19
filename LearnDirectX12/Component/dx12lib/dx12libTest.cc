#include "Adapter.h"
#include "Device.h"
#include "DescriptorAllocator.h"
#include "DescriptorAllocation.h"
#include "DescriptorAllocatorPage.h"
#include "CommandQueue.h"
#include "CommandListProxy.h"
#include "CommandList.h"
#include <iostream>

using namespace dx12lib;

void testDescriptorAllocator(std::shared_ptr<Device> pDevice) {
	auto descriptor0 = pDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 5);
	auto descriptor1 = pDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 45);
	auto descriptor2 = pDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 46);
}


struct Test {
	int a;
	char b;
	float c;
	double d;
};


void testStructConstantBuffer(std::shared_ptr<Device> pDevice) {
	auto pCmdQueue = pDevice->getCommandQueue(CommandQueueType::Direct);
	auto pCmdList = pCmdQueue->createCommandListProxy();
	auto pGPUTest = pCmdList->createStructConstantBuffer<Test>({ 10, 'a', 1.f, 10.0 });
	auto pMappedTest = pGPUTest->map();
	std::cout << pMappedTest->a << " " 
		      << pMappedTest->b << " "
		      << pMappedTest->c << " "
		      << pMappedTest->d << std::endl;

}

int main(int argc, char *argv[]) {
	std::shared_ptr<Adapter> pAdapter = std::make_shared<Adapter>();
	std::shared_ptr<Device> pDevice = std::make_shared<Device>(pAdapter);

	dx12lib::DeviceInitDesc desc = {
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_FORMAT_D24_UNORM_S8_UINT,
	};
	pDevice->initialize(desc);
	testDescriptorAllocator(pDevice);
	testStructConstantBuffer(pDevice);
	return 0;
}
