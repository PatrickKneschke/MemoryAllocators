
#include "free_list_allocator.h"
#include <stdexcept>


FreeListAllocator::FreeListAllocator(const size_t totalMemory) :
    IAllocator(totalMemory)
{
    pHead = new (pBase) FreeNode(mTotalMemory, nullptr);
}

FreeListAllocator::~FreeListAllocator() {

}

void* FreeListAllocator::Allocate(const size_t size, const size_t align) {
    
    // allow only power of 2 alignments
    assert( align > 0 && (align & (align - 1)) == 0);

    size_t requiredSize = size + sizeof(AllocHeader) + align - 1;
    
    FreeNode *curr = pHead, *prev = nullptr;
    while(curr && curr->size < requiredSize)
    {
        prev = curr;
        curr = curr->next;
    }

    if(curr == nullptr)
    {
        throw std::overflow_error("Freelist allocator does not have a large enough memory region available.");
    }

    size_t adjustment = align - (curr->address + sizeof(AllocHeader)) & (align - 1);
    // if current address is already properly aligned then adjustment = 0
    adjustment &= align - 1;

    uintptr_t alignedAddress = curr->address + adjustment + sizeof(AllocHeader);
    AllocHeader *header = reinterpret_cast<AllocHeader*>(alignedAddress - sizeof(AllocHeader));
    header->size = size;
    header->adjustment = adjustment;

    FreeNode *newNode = new (reinterpret_cast<void*>(alignedAddress + size)) FreeNode(curr->address + curr->size - alignedAddress - size, curr->next);
    prev->next = newNode;

    return reinterpret_cast<void*>(alignedAddress);
}

void FreeListAllocator::Free(void* ptr) {

    assert(ptr != nullptr);

    uintptr_t freeAddress = reinterpret_cast<uintptr_t>(ptr);
    AllocHeader *header = reinterpret_cast<AllocHeader*>( freeAddress - sizeof(AllocHeader) );
    freeAddress -= header->adjustment + sizeof(AllocHeader);
    
    FreeNode *newNode = new (reinterpret_cast<void*>(freeAddress)) FreeNode(header->size + header->adjustment + sizeof(AllocHeader));

    if(freeAddress < pHead->address)
    {
        newNode->next = pHead;
        pHead = newNode;
        return;
    }

    FreeNode *curr = pHead;
    while(curr->next && curr->next->address < freeAddress)
    {
        curr = curr->next;
    }
    newNode->next = curr->next;
    curr->next = newNode;
}

void FreeListAllocator::Clear() {
    
    pHead = new (pBase) FreeNode(mTotalMemory, nullptr);
    mUsedMemory = 0;
}