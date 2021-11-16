#include "BoxApp.h"

bool BoxApp::initialize() {
	if (!BaseApp::initialize())
		return false;

	buildDescriptorHeaps();
	buildConstantsBuffers();
	buildRootSignature();
	buildShaderAndInputLayout();
	buildBoxGeometry();
	buildPSO();

	ThrowIfFailed(pCommandList_->Reset(pCommandAlloc_.Get(), nullptr));
	ID3D12CommandList *cmdLists[] = { pCommandList_.Get() };
	pCommandQueue_->ExecuteCommandLists(1, cmdLists);
	flushCommandQueue();
	return true;
}
