
#include "stack_allocator.h"
#include <cstdint>
#include <stdexcept>


StackAllocator::StackAllocator(const size_t totalSize) :
    IAllocator(totalSize)
{
    mBaseAddress = reinterpret_cast<uintptr_t>(pBase);
    mTopAddress = mBaseAddress;

}

void* StackAllocator::Allocate(const size_t size, const size_t align = 0) {
    
    // allow only power of 2 alignments
    assert(align & (align -1) == 0);

    uint64_t adjustment = align - mTopAddress & (align - 1);
    // if mTopAddress is already properly aligned adjustment = 0
    adjustment &= align - 1;

    uintptr_t alignedAddress = mTopAddress + adjustment;
    if(alignedAddress - mBaseAddress + size > mTotalSize) 
    {
        throw std::overflow_error("Stack allocator out of memory!");
    }

    mTopAddress = alignedAddress + size;
    mUsed = mTopAddress - mBaseAddress;
    mMaxUsed = std::max(mMaxUsed, mUsed);

    return reinterpret_cast<void*>(alignedAddress);
}

void  StackAllocator::Free(void* ptr) {
    
    mTopAddress = reinterpret_cast<uintptr_t>(ptr);
    mUsed = mTopAddress - mBaseAddress;
}

void  StackAllocator::Clear() {

    mTopAddress = mBaseAddress;
    mUsed = 0;
}