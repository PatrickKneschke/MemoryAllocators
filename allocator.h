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

    /* @brief Constructor that allocates the managed memory portion.
     *
     * @param totalSize    The size of the allocated memory in bytes.
     */
    IAllocator(const size_t totalSize) : 
        mTotalSize {totalSize}, 
        mUsed {0}, 
        mMaxUsed {0} 
    { 
        pBase = std::malloc(mTotalSize);
    }

    /* @brief Default destructor that frees the allocated memory.
     */
    ~IAllocator() {

        std::free(pBase);
    }

    virtual void* Allocate(const size_t size, const size_t align = 0) = 0;
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

        void *mem = Allocate(sizeof(T));
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

        void *mem = Allocate(length * sizeof(T));
        for (size_t i = 0; i < length; i++)
        {    
            new (mem + i*sizeof(T)) T();
        }
    }

    /* @brief Delete object of type T.
     *
     * @param obj    Pointer to the object that should be deleted.
     */
    template<typename T>
    void Delete(const T* obj) {

        obj->~T();
        Free(static_cast<void*>(obj));
    }

    /* @brief Delete array of type T.
     *
     * @param arr    Pointer to the array that should be deleted.
     * @param length    Number of elements in the array.
     */
    template<typename T>
    void DeleteArr(const T* arr, const size_t length) {

        assert(length > 0);

        for (size_t i = 0; i < length; i++)
        {
            arr[i].~T();
        }
        
        Free(static_cast<void*>(arr));
    }


protected:

    // Pointer to the beginning of the allocated memory
    void *pBase;

    size_t mTotalSize;
    size_t mUsed;
    size_t mMaxUsed;

};