
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
    
    size_t requiredSize = size + sizeof(AllocHeader) + align - 1;

    // Find memory region large enough for allocation
    FreeNode *currNode = pHead, *prevNode = nullptr;
    while (currNode && currNode->size < requiredSize)
    {
        prevNode = currNode;
        currNode = currNode->next;
    }

    if (currNode == nullptr)
    {
        throw std::overflow_error("Freelist allocator does not have a large enough memory region available.");
    }

    // Find properly aligned address for allocation
    size_t adjustment = getAlignmentAdjustment(currNode->address + sizeof(AllocHeader), align);
    uintptr_t alignedAddress = currNode->address + adjustment + sizeof(AllocHeader);

    // Create a new node from remaining memory region of current node. 
    // If remaining memory is less than 24 byte ( 16 byte header plus 8 byte data) add it to the allocated memory section instead
    size_t newSize = currNode->address + currNode->size - alignedAddress - size;
    size_t allocSize = size;
    FreeNode *newNode = nullptr;
    if(newSize >= 24)
    {
        newNode = new (reinterpret_cast<void*>(alignedAddress + size)) FreeNode(newSize, currNode->next);
    }
    else
    {
        allocSize += newSize;
        newNode = currNode->next;
    }

    if (prevNode == nullptr)
    {
        pHead = newNode;
    }
    else
    {
        prevNode->next = newNode;
    }

    // Place allocation header in front of allocated memory section
    AllocHeader *header = reinterpret_cast<AllocHeader*>(alignedAddress - sizeof(AllocHeader));
    header->size = allocSize;
    header->adjustment = adjustment;

    mUsedMemory += header->adjustment + sizeof(AllocHeader) + header->size;
    mMaxUsedMemory = std::max(mMaxUsedMemory, mUsedMemory);

    return reinterpret_cast<void*>(alignedAddress);
}

void FreeListAllocator::Free(void* ptr) {

    assert(ptr != nullptr);

    // start address and size of free freed memory section
    uintptr_t freeAddress = reinterpret_cast<uintptr_t>(ptr);
    AllocHeader *header = reinterpret_cast<AllocHeader*>( freeAddress - sizeof(AllocHeader) );
    freeAddress -= header->adjustment + sizeof(AllocHeader);
    size_t freeSize = header->adjustment + sizeof(AllocHeader) + header->size;

    mUsedMemory -= freeSize;

    // find adjacent nodes
    FreeNode *nextNode = pHead, *prevNode = nullptr;
    while (nextNode && nextNode->address < freeAddress)
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