#pragma once


#include "allocator.h"


/* @brief Free list implementation of IAllocator.
 * 
 * Keeps track of unallocated memory regions with a list of FreeNodes, holding the size and address of the free region.
 * Allocates new memory from the first FreeNode large enough.
 * Frees memory by creating a new FreeNode in place of the allocated memory section or merges it with direct neighbors.
 * Clears all allocations by creating a new pHead FreeNode holding all the managed memory.
 * 
 * @class 
 */
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

    /* @brief Constructor that allocates the managed memory portion and calls Clear() method to reset free list.
     *
     * @param totalMemory    The size of the managed memory space in bytes.
     * @param parent    Optional parent allocator to get memory from.
     */
    explicit FreeListAllocator(const size_t totalMemory, IAllocator *parent = nullptr);

    /* @brief Default destructor that does nothing.
     */
    ~FreeListAllocator();
    
    /* @brief Allocates a properly aligned section of memory from the first FreeNode large enough. 
     *  
     * @param size    The size of the allocated memory section.
     * @param align    The alignment of the allocated memory section. Must be non-zero and a power of two.
     * 
     * return Pointer to the allocated memory.
     */
    void* Allocate(const size_t size, const size_t align = 1) override;

    /* @brief Frees the allocated memory section at ptr and creates a new FreeNode at that position or merges the new node with direct neighbors.
     * 
     * @param ptr    Pointer to the memory position to free.
     */
    void  Free(void* ptr) override;

    /* @brief Frees all the allocated memory by creating a new pHead FreeNode containing the whole memory.
     */
    void  Clear() override;


private:

    FreeNode* pHead;
};