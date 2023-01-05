#pragma once


#include "allocator.h"


class StackAllocator : public IAllocator{

public:

    StackAllocator() = delete;
    explicit StackAllocator(const size_t totalSize);
    
    void* Allocate(const size_t size, const size_t align = 0) override;
    void  Free(void* ptr) override;
    void  Clear() override;


private:

    uintptr_t mBaseAddress;
    uintptr_t mTopAddress;
};