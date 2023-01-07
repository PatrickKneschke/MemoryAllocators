#pragma once


#include "allocator.h"
#include <cstdint>


class StackAllocator : public IAllocator{

public:

    StackAllocator() = delete;
    explicit StackAllocator(const size_t totalMemory);

    ~StackAllocator();
    
    void* Allocate(const size_t size, const size_t align = 1) override;
    void  Free(void* ptr) override;
    void  Clear() override;


private:

    uintptr_t mBaseAddress;
    uintptr_t mTopAddress;
};