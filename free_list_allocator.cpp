
#include "free_list_allocator.h"
#include <stdexcept>

#include <iostream>


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
    
    // find large enough memory region for allocation
    FreeNode *curr = pHead, *prev = nullptr;
    while (curr && curr->size < requiredSize)
    {
        prev = curr;
        curr = curr->next;
    }

    if (curr == nullptr)
    {
        throw std::overflow_error("Freelist allocator does not have a large enough memory region available.");
    }

    size_t adjustment = align - (curr->address + sizeof(AllocHeader)) & (align - 1);
    // if current address is already properly aligned then adjustment = 0
    adjustment &= align - 1;

    // create a new node from remaining memory region of current node
    uintptr_t alignedAddress = curr->address + adjustment + sizeof(AllocHeader);
    FreeNode *newNode = new (reinterpret_cast<void*>(alignedAddress + size)) FreeNode(curr->address + curr->size - alignedAddress - size, curr->next);

    if (prev == nullptr)
    {
        pHead = newNode;
    }
    else
    {
        prev->next = newNode;
    }

    // place allocation header in front of allocated memory section
    AllocHeader *header = reinterpret_cast<AllocHeader*>(alignedAddress - sizeof(AllocHeader));
    header->size = size;
    header->adjustment = adjustment;

    return reinterpret_cast<void*>(alignedAddress);
}

void FreeListAllocator::Free(void* ptr) {

    assert(ptr != nullptr);

    // start address and size of free freed memory section
    uintptr_t freeAddress = reinterpret_cast<uintptr_t>(ptr);
    AllocHeader *header = reinterpret_cast<AllocHeader*>( freeAddress - sizeof(AllocHeader) );
    freeAddress -= header->adjustment + sizeof(AllocHeader);
    size_t freeSize = header->adjustment + sizeof(AllocHeader) + header->size;

    // find adjacent nodes
    FreeNode *nextNode = pHead, *prevNode = nullptr;
    while (nextNode->address < freeAddress)
    {
        prevNode = nextNode;
        nextNode = nextNode->next;
    }

    // combine freed memory section with adjacent nodes if necessary
    if (prevNode && prevNode->address + prevNode->size == freeAddress)
    {
        freeAddress = prevNode->address;
        freeSize += prevNode->size;
    }
    if (nextNode && nextNode->address == freeAddress + freeSize)
    {
        freeSize += nextNode->size;
        nextNode = nextNode->next;
    }
    
    // create new node for freed section, this may override prevNode, but all pointers are still valid 
    FreeNode *newNode = new (reinterpret_cast<void*>(freeAddress)) FreeNode(freeSize, nextNode);
    
    if (prevNode == nullptr)
    {
        pHead = newNode;
        return;
    }
    if (prevNode != newNode)
    {
        prevNode->next = newNode;
    }
}

void FreeListAllocator::Clear() {
    
    pHead = new (pBase) FreeNode(mTotalMemory, nullptr);
    mUsedMemory = 0;
}