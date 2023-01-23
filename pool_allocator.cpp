
#include "pool_allocator.h"
#include <stdexcept>


PoolAllocator::PoolAllocator(const size_t totalMemory, const size_t chunkSize, IAllocator *parent) :
    IAllocator(totalMemory, parent),
    mChunkSize {chunkSize}
{
    assert(totalMemory % chunkSize == 0);

    mNumChunks = totalMemory / chunkSize;
    Clear();   
}

PoolAllocator::~PoolAllocator() {

}

void* PoolAllocator::Allocate(const size_t size, const size_t align) {

    assert(size <= mChunkSize);
    assert(mChunkSize % align == 0);

    if(!pHead)
    {
        throw std::overflow_error("Pool allocator is out of memory!");
    }

    void *mem = reinterpret_cast<void*>(pHead);
    pHead = pHead->next;
    
    mUsedMemory += mChunkSize;
    mMaxUsedMemory = std::max(mMaxUsedMemory, mUsedMemory);

    return mem;
}

void PoolAllocator::Free(void* ptr) {

    assert(ptr != nullptr);

    pHead = new (ptr) PoolNode(pHead);
    
    mUsedMemory -= mChunkSize;
}

void PoolAllocator::Clear() {

    pHead = nullptr;
    uintptr_t address = reinterpret_cast<uintptr_t>(pBase) + mTotalMemory;
    for (size_t i = 0; i < mNumChunks; i++)
    {
        address -= mChunkSize;
        pHead = new (reinterpret_cast<void*>(address)) PoolNode(pHead);
    }

    mUsedMemory = 0;
}