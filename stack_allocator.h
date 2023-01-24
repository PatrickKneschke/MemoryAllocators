#pragma once


#include "allocator.h"


/* @brief Stack implementation of IAllocator.
 * 
 * Allocates new memory from mTopAddress of the used memory region.
 * Frees memory by moving mTopAddress of the used memory region down to a specific address, freeing all allocated memory above.
 * Clears all allocations by setting mTopAddress to mBaseAddress of the managed memory space.
 * 
 * @class 
 */
class StackAllocator : public IAllocator{

public:

    StackAllocator() = delete;

    /* @brief Constructor that allocates the managed memory portion.
     *
     * @param totalMemory    The size of the managed memory space in bytes.
     * @param parent    Optional parent allocator to get memory from.
     */
    explicit StackAllocator(const size_t totalMemory, IAllocator *parent = nullptr);

    /* @brief Default destructor that does nothing.
     */
    ~StackAllocator();
    
    /* @brief Allocates a properly aligned section of memory from the top of the stack. 
     *  
     * @param size    The size of the allocated memory section.
     * @param align    The alignment of the allocated memory section. Must be non-zero and a power of two.
     * 
     * return Pointer to the allocated memory.
     */
    void* Allocate(const size_t size, const size_t align = 1) override;

    /* @brief Frees all the allocated memory from the top of the stack down to a given position.
     * 
     * @param ptr    Pointer to the memory position to free.
     */
    void  Free(void* ptr) override;

    /* @brief Frees all the allocated memory of the stack.
     */
    void  Clear() override;


private:

    uintptr_t mBaseAddress;
    uintptr_t mTopAddress;
};