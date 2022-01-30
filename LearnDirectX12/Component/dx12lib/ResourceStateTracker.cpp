#include "IResource.h"
#include "ResourceStateTracker.h"
#include "CommandList.h"
#include "CommandListProxy.h"

namespace dx12lib {

void ResourceStateTracker::resourceBarrier(const D3D12_RESOURCE_BARRIER &expectBarrier) {
	if (expectBarrier.Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION) {
		const D3D12_RESOURCE_TRANSITION_BARRIER &expectTransition = expectBarrier.Transition;
		const auto iter = _finalResourceState.find(expectTransition.pResource);
		if (iter != _finalResourceState.end()) {	// 状态已知的情况
			auto &finalResourceState = iter->second;
			// 内部的 subResource 不统一
			if (expectTransition.Subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES &&
				!finalResourceState._subresourceState.empty())
			{
				for (auto subresourceState : finalResourceState._subresourceState) {
					if (expectTransition.StateAfter != subresourceState.second) {
						D3D12_RESOURCE_BARRIER newBarrier = expectBarrier;
						newBarrier.Transition.Subresource = subresourceState.first;
						newBarrier.Transition.StateBefore = subresourceState.second;
						_resourceBarriers.push_back(newBarrier);
					}
				}
			} else {	// 整个 resource 都转换为 expectBarrier 状态
				auto finalState = finalResourceState.getSubresourceState(expectTransition.Subresource);
				if (finalState != expectTransition.StateAfter) {
					D3D12_RESOURCE_BARRIER newBarrier = expectBarrier;
					newBarrier.Transition.StateBefore = finalState;
					_resourceBarriers.push_back(newBarrier);
				}
			}
		} else {	
			_pendingResourceBarriers.push_back(expectBarrier);
		}
		auto &finalResourceStateRecord = _finalResourceState[expectTransition.pResource];
		finalResourceStateRecord.setSubresourceState(expectTransition.Subresource, expectTransition.StateAfter);
	} else {
		_resourceBarriers.push_back(expectBarrier);
	}
}

void ResourceStateTracker::transitionResource(ID3D12Resource *pResource, 
	D3D12_RESOURCE_STATES stateAfter, 
	UINT subResource /*= D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES */) 
{
	resourceBarrier(CD3DX12_RESOURCE_BARRIER::Transition(
		pResource,
		D3D12_RESOURCE_STATE_COMMON,
		stateAfter,
		subResource
	));
}

void ResourceStateTracker::transitionResource(const IResource &resource, 
	D3D12_RESOURCE_STATES stateAfter, 
	UINT subResource /*= D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES */) 
{
	resourceBarrier(CD3DX12_RESOURCE_BARRIER::Transition(
		resource.getD3DResource().Get(),
		D3D12_RESOURCE_STATE_COMMON,
		stateAfter,
		subResource
	));
}

void ResourceStateTracker::UAVBarrier(const IResource *pResource /*= nullptr*/) {
	auto *pD3DResource = pResource != nullptr ? pResource->getD3DResource().Get() : nullptr;
	resourceBarrier(CD3DX12_RESOURCE_BARRIER::UAV(pD3DResource));
}

void ResourceStateTracker::aliasBarrier(const IResource *pResourceBefore /*= nullptr*/, 
	const IResource *pResourceAfter /*= nullptr*/) 
{
	auto *pD3DBeforceResource = pResourceBefore != nullptr ? pResourceBefore->getD3DResource().Get() : nullptr;
	auto *pD3DAfterResource = pResourceAfter != nullptr ? pResourceAfter->getD3DResource().Get() : nullptr;
	resourceBarrier(CD3DX12_RESOURCE_BARRIER::Aliasing(pD3DBeforceResource, pD3DAfterResource));
}

uint32 ResourceStateTracker::flushResourceBarriers(CommandListProxy pCmdList) {
	UINT numBarrier = static_cast<UINT>(_resourceBarriers.size());
	if (numBarrier > 0) {
		pCmdList->getD3DCommandList()->ResourceBarrier(numBarrier, _resourceBarriers.data());
		_resourceBarriers.clear();
	}
}

void ResourceStateTracker::commitFinalResourceStates() {
	assert(_isLocked);
	for (const auto &resourceState : _finalResourceState) 
		_globalResourceState[resourceState.first] = resourceState.second;
	_finalResourceState.clear();
}

void ResourceStateTracker::flusePendingResourceBarriers(CommandListProxy pCmdList) {
	assert(_isLocked);

}

void ResourceStateTracker::reset() {
	_pendingResourceBarriers.clear();
	_resourceBarriers.clear();
	_finalResourceState.clear();
}

void ResourceStateTracker::lock() {
	_globalMutex.lock();
	_isLocked = true;
}

void ResourceStateTracker::unlock() {
	_globalMutex.unlock();
	_isLocked = false;
}

void ResourceStateTracker::addGlobalResourceState(ID3D12Resource *pResource, D3D12_RESOURCE_STATES state) {
	if (pResource != nullptr) {
		std::lock_guard lock(_globalMutex);
		_globalResourceState[pResource].setSubresourceState(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, state);
	}
}

void ResourceStateTracker::removeGlobalResourceState(ID3D12Resource *pResource) {
	if (pResource != nullptr) {
		std::lock_guard lock(_globalMutex);
		_globalResourceState.erase(pResource);
	}
}

}