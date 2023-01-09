#pragma once


#include "allocator.h"


class StackAllocator : public IAllocator{

public:

    StackAllocator() = delete;

    /* @brief Constructor that allocates the managed memory portion.
     *
     * @param totalMemory    The size of the managed memory space in bytes.
     */
    explicit StackAllocator(const size_t totalMemory);

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