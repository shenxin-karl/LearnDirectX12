#pragma once
#include "dx12libStd.h"

namespace dx12lib {
	
class ResourceStateTracker {
protected:
	ResourceStateTracker() = default;
public:
	~ResourceStateTracker() = default;
	void resourceBarrier(const D3D12_RESOURCE_BARRIER &barrier);
	void transitionResource(ID3D12Resource *pResource, 
		D3D12_RESOURCE_STATES stateAfter, 
		UINT subResource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES
	);
	void transitionResource(const IResource &resource,
		D3D12_RESOURCE_STATES stateAfter,
		UINT subResource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES
	);
	void UAVBarrier(const IResource *resource = nullptr);
	void aliasBarrier(const IResource *resourceBefore = nullptr, const IResource *resourceAfter = nullptr);
	uint32 flushResourceBarriers(std::shared_ptr<CommandList> commandList);
	void commitFinalResourceStates();
	UINT flusePendingResourceBarriers(std::shared_ptr<CommandList> pCmdList);
	void reset();
	static void lock();
	static void unlock();
	static void addGlobalResourceState(ID3D12Resource *pResource, D3D12_RESOURCE_STATES state);
	static void removeGlobalResourceState(ID3D12Resource *pResource);
private:
	struct ResourceState {
		explicit ResourceState(D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON);
		void setSubresourceState(UINT subresource, D3D12_RESOURCE_STATES state);
		D3D12_RESOURCE_STATES getSubresourceState(UINT subresource);
	public:
		D3D12_RESOURCE_STATES                  _state;
		std::map<UINT, D3D12_RESOURCE_STATES>  _subresourceState;
	};
	using ResourceBarriers = std::vector<D3D12_RESOURCE_BARRIER>;
	using ResourceStateMap = std::unordered_map<ID3D12Resource *, ResourceState>;
private:
	ResourceBarriers  _pendingResourceBarriers;
	ResourceBarriers  _resourceBarriers;
	ResourceStateMap  _finalResourceState;

	static inline bool              _isLocked = false;
	static inline std::mutex        _globalMutex;
	static inline ResourceStateMap  _globalResourceState;
};

}