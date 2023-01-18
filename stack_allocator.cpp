
#include "stack_allocator.h"
#include <stdexcept>


StackAllocator::StackAllocator(const size_t totalMemory, IAllocator *parent) :
    IAllocator(totalMemory, parent)
{
    mBaseAddress = reinterpret_cast<uintptr_t>(pBase);
    mTopAddress = mBaseAddress;
}

StackAllocator::~StackAllocator() {

}

void* StackAllocator::Allocate(const size_t size, const size_t align) {
    
    size_t adjustment = getAlignmentAdjustment(mTopAddress, align);

    uintptr_t alignedAddress = mTopAddress + adjustment;
    if (alignedAddress - mBaseAddress + size > mTotalMemory) 
    {
        throw std::overflow_error("Stack allocator is out of memory!");
    }

    mTopAddress = alignedAddress + size;
    mUsedMemory = mTopAddress - mBaseAddress;
    mMaxUsedMemory = std::max(mMaxUsedMemory, mUsedMemory);

    return reinterpret_cast<void*>(alignedAddress);
}

void StackAllocator::Free(void* ptr) {

    assert(ptr != nullptr);
    
    uintptr_t newTopAddress = reinterpret_cast<uintptr_t>(ptr);

    // Do nothing if attempt is made to free memory outside used memory range
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