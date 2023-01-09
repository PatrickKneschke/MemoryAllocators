
#include "stack_allocator.h"
#include <stdexcept>


StackAllocator::StackAllocator(const size_t totalMemory) :
    IAllocator(totalMemory)
{
    mBaseAddress = reinterpret_cast<uintptr_t>(pBase);
    mTopAddress = mBaseAddress;
}

StackAllocator::~StackAllocator() {

}

void* StackAllocator::Allocate(const size_t size, const size_t align) {
    
    // allow only power of 2 alignments
    assert( align > 0 && (align & (align - 1)) == 0);

    size_t adjustment = align - mTopAddress & (align - 1);
    // if mTopAddress is already properly aligned then adjustment = 0
    adjustment &= align - 1;

    uintptr_t alignedAddress = mTopAddress + adjustment;
    if (alignedAddress - mBaseAddress + size > mTotalMemory) 
    {
        throw std::overflow_error("Stack allocator out of memory!");
    }

    mTopAddress = alignedAddress + size;
    mUsedMemory = mTopAddress - mBaseAddress;
    mMaxUsedMemory = std::max(mMaxUsedMemory, mUsedMemory);

    return reinterpret_cast<void*>(alignedAddress);
}

void StackAllocator::Free(void* ptr) {

    assert(ptr != nullptr);
    
    uintptr_t newTopAddress = reinterpret_cast<uintptr_t>(ptr);

    //Do nothing if attemt is made to free memory outside used memory range
    if (mTopAddress <= newTopAddress)
    {
        return;
    }

    mTopAddress = newTopAddress;
    mUsedMemory = mTopAddress - mBaseAddress;
}

void StackAllocator::Clear() {

    mTopAddress = mBaseAddress;
    mUsedMemory = 0;
}