#pragma once


#include "allocator.h"
#include <cstdint>


class FreeListAllocator : public IAllocator{

    struct FreeNode {

        size_t size;
        uintptr_t address;
        FreeNode *next;

        FreeNode() : size {0}, address {0UL}, next {nullptr} {}
        FreeNode(const size_t size_, FreeNode *next_ = nullptr) : size {size_}, next {next_} {

            address = reinterpret_cast<uintptr_t>(this);
        }
    };

    struct AllocHeader {

        size_t size;
        size_t adjustment;

        AllocHeader(const size_t size_, const size_t adjustment_) : size {size_}, adjustment {adjustment} {}
    };

public:

    FreeListAllocator() = delete;
    explicit FreeListAllocator(const size_t totalMemory);

    ~FreeListAllocator();
    
    
    void* Allocate(const size_t size, const size_t align = 1) override;

    void  Free(void* ptr) override;

    void  Clear() override;


private:

    FreeNode* pHead;
};