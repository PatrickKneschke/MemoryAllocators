
#include "pool_allocator.h"
#include <stdexcept>

#include <iostream>


PoolAllocator::PoolAllocator(const size_t totalMemory, const size_t nodeSize) :
    IAllocator(totalMemory),
    mNodeSize {nodeSize}
{
    assert(totalMemory % nodeSize == 0);
    mNumNodes = totalMemory / nodeSize;

    pHead = nullptr;
    uintptr_t address = reinterpret_cast<uintptr_t>(pBase) + mTotalMemory;
    for (size_t i = 0; i < mNumNodes; i++)
    {
        address -= nodeSize;
        pHead = new (reinterpret_cast<void*>(address)) PoolNode(pHead);
    }    
}

PoolAllocator::~PoolAllocator() {

}

void* PoolAllocator::Allocate(const size_t size, const size_t align) {

    assert(size <= mNodeSize);

    if(!pHead) {

        throw std::overflow_error("Pool allocator is out of memory!");
    }

    void *mem = reinterpret_cast<void*>(pHead);
    pHead = pHead->next;
    
    mUsedMemory += mNodeSize;
    mMaxUsedMemory = std::max(mMaxUsedMemory, mUsedMemory);

    return mem;
}

void PoolAllocator::Free(void* ptr) {

    assert(ptr != nullptr);

    pHead = new (ptr) PoolNode(pHead);
    
    mUsedMemory -= mNodeSize;
}

void PoolAllocator::Clear() {
    
    pHead = new (pBase) PoolNode();
    mUsedMemory = 0;
}