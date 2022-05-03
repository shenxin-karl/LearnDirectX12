#include <iostream>
#include "IResource.h"
#include "ResourceStateTracker.h"
#include "CommandList.h"
#include "ContextProxy.hpp"

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

void ResourceStateTracker::UAVBarrier(const IResource *pResource /*= nullptr*/) {
	auto *pD3DResource = pResource != nullptr ? pResource->getD3DResource().Get() : nullptr;
	assert(pD3DResource != nullptr);
	resourceBarrier(CD3DX12_RESOURCE_BARRIER::UAV(pD3DResource));
}

void ResourceStateTracker::aliasBarrier(const IResource *pBeforce, const IResource *pAfter) {
	auto *pD3DBeforceResource = pBeforce != nullptr ? pBeforce->getD3DResource().Get() : nullptr;
	auto *pD3DAfterResource = pAfter != nullptr ? pAfter->getD3DResource().Get() : nullptr;
	if (pD3DBeforceResource == nullptr || pD3DAfterResource == nullptr) {
		assert(false);
		std::cout << __FUNCTION__ << " pD3DBeforceResource == nullptr || pD3DAfterResource == nullptr" << std::endl;
		return;
	}
	resourceBarrier(CD3DX12_RESOURCE_BARRIER::Aliasing(pD3DBeforceResource, pD3DAfterResource));
}

uint32 ResourceStateTracker::flushResourceBarriers(std::shared_ptr<CommandList> pCmdList) {
	UINT numBarrier = static_cast<UINT>(_resourceBarriers.size());
	if (numBarrier > 0) {
		pCmdList->getD3DCommandList()->ResourceBarrier(numBarrier, _resourceBarriers.data());
		_resourceBarriers.clear();
	}
	return numBarrier;
}

void ResourceStateTracker::commitFinalResourceStates() {
	assert(_isLocked);
	for (auto &&[pResource, state] : _finalResourceState) 
		_globalResourceState[pResource] = state;
	_finalResourceState.clear();
}

UINT ResourceStateTracker::flusePendingResourceBarriers(std::shared_ptr<CommandList> pCmdList) {
	assert(_isLocked);

	std::vector<D3D12_RESOURCE_BARRIER> resourceBarriers;
	resourceBarriers.reserve(_pendingResourceBarriers.size());
	for (auto &pendingBarrier : _pendingResourceBarriers) {
		if (pendingBarrier.Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION) {
			const auto &pendingTransition = pendingBarrier.Transition;
			auto iter = _globalResourceState.find(pendingTransition.pResource);
			if (iter == _globalResourceState.end())
				continue;

			const auto &currResourceState = iter->second;
			if (pendingTransition.Subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES &&
				!currResourceState._subresourceState.empty())
			{
				for (auto &&[subResource, state] : currResourceState._subresourceState) {
					if (pendingTransition.StateAfter != state) {
						auto newBarrier = pendingBarrier;
						newBarrier.Transition.Subresource = subResource;
						newBarrier.Transition.StateBefore = state;
						resourceBarriers.push_back(newBarrier);
					}
				}
			} else {	// Uniform subResource status	
				if (currResourceState._state != pendingTransition.StateAfter) {
					auto newBarrier = pendingBarrier;
					newBarrier.Transition.StateBefore = currResourceState._state;
					resourceBarriers.push_back(newBarrier);
				}
			}
		}
	}
	_pendingResourceBarriers.clear();

	UINT numBarriers = static_cast<UINT>(resourceBarriers.size());
	if (numBarriers > 0) {
		pCmdList->getD3DCommandList()->ResourceBarrier(
			numBarriers, 
			resourceBarriers.data()
		);
	}
	return numBarriers;
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

ResourceStateTracker::ResourceState::ResourceState(D3D12_RESOURCE_STATES state /*= D3D12_RESOURCE_STATE_COMMON*/)
: _state(state) {
}

void ResourceStateTracker::ResourceState::setSubresourceState(UINT subresource, D3D12_RESOURCE_STATES state) {
	if (subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES) {
		_state = state;
		_subresourceState.clear();
	} else {
		_subresourceState[subresource] = state;
	}
}

D3D12_RESOURCE_STATES ResourceStateTracker::ResourceState::getSubresourceState(UINT subresource) {
	auto state = _state;
	if (auto iter = _subresourceState.find(subresource); iter != _subresourceState.end())
		state = iter->second;
	return state;
}

}