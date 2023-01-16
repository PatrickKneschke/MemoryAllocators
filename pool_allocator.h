#pragma once


#include "allocator.h"


/* @brief Pool implementation of IAllocator.
 * 
 * Splits the managed memory space into chunks of equal size and Keeps track of unallocated memory regions with a linked list of PoolNodes.
 * Allocates new memory from pHead of the list.
 * Frees memory by creating a new PoolNode in place of the allocated memory section and makes it the new pHead.
 * Clears all allocations by creatig a new list of PoolNodes covering the whole memory space.
 * 
 * @class 
 */
class PoolAllocator : public IAllocator{

    struct PoolNode {

        PoolNode *next;

        PoolNode(PoolNode *next_ = nullptr) : next {next_} {}
    };

public:

    PoolAllocator() = delete;

    /* @brief Constructor that allocates the managed memory portion and splits it into chunks. Creates a linked list of PoolNodes to track free chunks.
     *
     * @param totalMemory    The size of the managed memory space in bytes.
     * @param chunkSize    The size of each allocatable memory region.
     */
    explicit PoolAllocator(const size_t totalMemory, const size_t chunkSize);

    /* @brief Default destructor that does nothing.
     */
    ~PoolAllocator();
    
    /* @brief Allocates a properly aligned section of memory from pHead. 
     *  
     * @param size    The size of the allocated memory section.
     * @param align    The alignment of the allocated memory section. Must be non-zero and a power of two.
     * 
     * return Pointer to the allocated memory.
     */
    void* Allocate(const size_t size, const size_t align = 1) override;

    /* @brief Frees the allocated memory section at ptr and creates a new pHead PoolNode at that position.
     * 
     * @param ptr    Pointer to the memory position to free.
     */
    void  Free(void* ptr) override;

    /* @brief Frees all the allocated memory by creating a new list of PoolNodes covering the whole memory space.
     */
    void  Clear() override;


private:

    PoolNode* pHead;
    size_t mChunkSize;
    size_t mNumChunks;
};