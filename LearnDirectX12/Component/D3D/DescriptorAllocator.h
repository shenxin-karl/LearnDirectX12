#pragma once
#include <memory>
#include <mutex>
#include <vector>
#include <map>
#include <set>
#include <queue>
#include "d3dutil.h"

namespace d3dUtil {


class DescriptorAllocation;
class DescriptorAllocatorPage;
class DescriptorAllocator;

class DescriptorAllocation {
public:
    DescriptorAllocation();
    DescriptorAllocation(D3D12_CPU_DESCRIPTOR_HANDLE descriptor, 
        uint32 numHandles, 
        uint32 descriptorSize,
        uint64 frameNumber,
        std::shared_ptr<DescriptorAllocatorPage> pPage
    );
    DescriptorAllocation(const DescriptorAllocation &) = delete;
    DescriptorAllocation(DescriptorAllocation &&other) noexcept;
    DescriptorAllocation &operator=(DescriptorAllocation &&other) noexcept;
    ~DescriptorAllocation();
    bool isNull() const noexcept;
    D3D12_CPU_DESCRIPTOR_HANDLE getDescriptorHandle(uint32 offset = 0) const noexcept;
    uint32 getNumHandles() const noexcept;
    uint64 getFrameNumber() const noexcept;
    auto getDescriptorAllocatorPage() const noexcept;
    friend void swap(DescriptorAllocation &lhs, DescriptorAllocation &rhs) noexcept;
private:
    void free();
private:
    uint32  _numDescriptors;
    uint32  _descriptorSize;
    uint64  _frameNumber;
    D3D12_CPU_DESCRIPTOR_HANDLE  _descriptor;
    std::shared_ptr<DescriptorAllocatorPage>  _page;
};

class DescriptorAllocatorPage : public std::enable_shared_from_this<DescriptorAllocatorPage> {
public:
    DescriptorAllocatorPage(ID3D12Device *pDevice, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32 numDescriptors);
    bool hasSpace(uint32 numDescriptors) const noexcept;
    uint32 numFreeHandles() const noexcept;
    DescriptorAllocation allocate(uint32 numDescriptors, uint64 frameNumber);
    void free(DescriptorAllocation &&handle, uint64 frameNumber);
    void releaseStaleDescriptor(uint64 frameNumber);
protected:
    uint32 computeOffset(D3D12_CPU_DESCRIPTOR_HANDLE handle);
    void addNewBlock(uint32 offset, uint32 numDescriptors);
    void freeBlock(uint32 offset, uint32 numDescriptors);
private:
    struct FreeBlockInfo;

    using OffsetType = uint32;
    using SizeType = uint32;

    using FreeListByOffset = std::map<OffsetType, FreeBlockInfo>;
    using FreeListBySize = std::multimap<SizeType, FreeListByOffset::iterator>;

    struct FreeBlockInfo {
        SizeType  _size;
        FreeListBySize::iterator  _freeListBySizeIt;
    };

    struct StaleDescripotrInfo {
        OffsetType  _offset;
        SizeType    _size;
        uint64      _frameNumber;
    };

    using StaleDescripotrQueue = std::queue<StaleDescripotrInfo>;
private:
    FreeListByOffset      _freeListByOffset;
    FreeListBySize        _freeListBySize;
    StaleDescripotrQueue  _staleDescriptorQueue;

    WRL::ComPtr<ID3D12DescriptorHeap>  _pDescriptorHeap;
    D3D12_DESCRIPTOR_HEAP_TYPE         _heapType;
    CD3DX12_CPU_DESCRIPTOR_HANDLE      _baseDescriptor;
    std::mutex                         _allocationMutex;
    uint32                           _descriptorHandleIncrmenetSize;
    uint32                           _numDescriptorInHeap;
    uint32                           _numFreeHandles;
    uint64                           _frameNumber;
};

class DescriptorAllocator {
public:
    DescriptorAllocator(ID3D12Device *pDevice, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32 numDescriptorsPreHeap);
    DescriptorAllocator(const DescriptorAllocator &) = delete;
    DescriptorAllocator(DescriptorAllocator &&other) = default;
    DescriptorAllocation allocate(uint32 numDescriptors, uint64 frameNumber);
    void releaseStaleDescriptors(uint64 frameNumber);
    ~DescriptorAllocator() = default;
private:
    using HeapPagePool = std::vector<std::shared_ptr<DescriptorAllocatorPage>>;
    HeapPagePool                    _heapPool;
    uint32                          _numDescriptorsPreHeap;
    std::mutex                      _allocationMutex;
    std::set<std::size_t>           _availableHeaps;
    D3D12_DESCRIPTOR_HEAP_TYPE      _heapType;
    ID3D12Device                   *_pDevice;
};

}

