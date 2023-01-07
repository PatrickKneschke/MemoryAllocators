#pragma once


#include <cassert>
#include <cstdlib>


/* @brief Allocator base class. 
 * @class 
 *
 * Abstract base class for allocators used to manage a large portion of memory.
 * Provides implementation of New and Delete for single objects and arrays, shared by all derived allocators.
 * Provides interface methods to Allocate and Free portions of memory and Clear the entire memory, to be implemented by derived allocators.
 */
class IAllocator {

public:

    IAllocator() = delete;

    /* @brief Constructor that allocates the managed memory portion.
     *
     * @param totalMemory    The size of the allocated memory in bytes.
     */
    IAllocator(const size_t totalMemory) : 
        mTotalMemory {totalMemory}, 
        mUsedMemory {0}, 
        mMaxUsedMemory {0} 
    { 
        pBase = std::malloc(mTotalMemory);
    }

    /* @brief Default destructor that frees the allocated memory.
     */
    ~IAllocator() {

        std::free(pBase);
    }

    virtual void* Allocate(const size_t size, const size_t align = 1) = 0;
    virtual void  Free(void* ptr) = 0;
    virtual void  Clear() = 0;

    /* @brief Initialize new object of type T.
     *
     * @param args    Argument list for constructor.
     * 
     * @return Pointer to the initialized object.
     */
    template<typename T, typename... Args>
    T* New(Args... args) {

        void *mem = Allocate(sizeof(T), alignof(T));
        return new (mem) T(args...);
    }

    /* @brief Initialize new array of type T.
     *
     * @param length    Number of elements in the array.
     * 
     * @return Pointer to the initialized array.
     */
    template<typename T>
    T* NewArr(const size_t length) {

        assert(length > 0);

        T *mem = static_cast<T*>(Allocate(length * sizeof(T), alignof(T)));
        for (size_t i = 0; i < length; i++)
        {    
            new (mem + i) T();
        }

        return mem;
    }

    /* @brief Delete object of type T.
     *
     * @param obj    Pointer to the object that should be deleted.
     */
    template<typename T>
    void Delete(T* obj) {

        obj->~T();
        Free(static_cast<void*>(obj));
    }

    /* @brief Delete array of type T.
     *
     * @param arr    Pointer to the array that should be deleted.
     * @param length    Number of elements in the array.
     */
    template<typename T>
    void DeleteArr(T* arr, const size_t length) {

        assert(length > 0);

        for (size_t i = 0; i < length; i++)
        {
            arr[i].~T();
        }
        
        Free(static_cast<void*>(arr));
    }


    size_t  totalMemory()   const { return mTotalMemory;}
    size_t  usedMemory()    const { return mUsedMemory;}
    size_t  maxUsedMemory() const { return mMaxUsedMemory;}

protected:

    // Pointer to the beginning of the allocated memory
    void *pBase;

    size_t mTotalMemory;
    size_t mUsedMemory;
    size_t mMaxUsedMemory;

};