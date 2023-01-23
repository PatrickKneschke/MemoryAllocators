
#include "free_list_allocator.h"
#include "free_tree_allocator.h"
#include "pool_allocator.h"
#include "stack_allocator.h"

#include <chrono>
#include <iostream>
#include <string>
#include <queue>
#include <vector>


using Clock = std::chrono::high_resolution_clock;
using Time  = std::chrono::time_point<std::chrono::high_resolution_clock>;


int64_t duration(Time start, Time end) {

    return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}


void benchmarkStack(size_t totalMemory) {

    std::vector<size_t> allocationSizes = {16, 64, 256, 1024, 4096, 16384};  
    std::vector<size_t> numAllocations(allocationSizes.size());
   
    size_t numOperations = 0;
    for(size_t i = 0; i < allocationSizes.size(); i++)
    {
        numAllocations[i] = totalMemory / allocationSizes[i];
        numOperations += numAllocations[i];
    }

    // test stack allocator
    StackAllocator stAlloc(totalMemory);

    Clock clock;    
    Time start = clock.now();

    for(size_t i = 0; i < numAllocations.size(); i++)
    {
        for (size_t j = 0; j < numAllocations[i]; j++)
        {
            void *ptr = stAlloc.Allocate(allocationSizes[i]);
        }
        stAlloc.Clear();
    }

    Time end = clock.now();

    std::cout << "\nStackAllocator : " << numOperations << " operations in " << duration(start, end) / 1000000.0 << " s" << '\n';

    // test malloc
    start = clock.now();

    for(size_t i = 0; i < numAllocations.size(); i++)
    {
        for (size_t j = 0; j < numAllocations[i] - 1; j++)
        {
            void *ptr = malloc(allocationSizes[i]);
        }
    }

    end = clock.now();

    std::cout << "malloc : " << numOperations << " in " << duration(start, end) / 1000000.0 << " s" << '\n';    
}


void benchmarkList(size_t totalMemory) {
  
    std::vector<size_t> allocationSizes = {16, 64, 256, 1024, 4096, 16384};    
    size_t numOperations = 10000000;
    std::queue<void*> ptrs;

    srand(time(nullptr));

    // test stack allocator
    FreeListAllocator listAlloc(totalMemory);

    Clock clock;    
    Time start = clock.now();

    // calls to Allocate() and Free() with random sizes
    // if out of memory do up to 10 calls to Free() to create space
    for (size_t i = 0; i < numOperations; i++)
    {
        if(rand() % 5 == 0 && !ptrs.empty())
        {
            listAlloc.Free(ptrs.front());
            ptrs.pop();
            continue;
        }
         
        int r = rand() % 6;
        try
        {
            void *p = listAlloc.Allocate(allocationSizes[r]);
            ptrs.push(p);
        }
        catch(const std::exception& e)
        {
            for (size_t j = 0; j < 10; j++)
            {
                if(ptrs.empty())
                {
                    break;
                }
                    
                listAlloc.Free(ptrs.front());
                ptrs.pop();
                ++i;
            }            
        }        
    }
    

    Time end = clock.now();

    std::cout << "\nFreeListAllocator : " << numOperations << " operations in " << duration(start, end) / 1000000.0 << " s" << '\n';

    while(!ptrs.empty())
    {
        ptrs.pop();
    }
    
    // test malloc
    // calls to malloc() and free() with random sizes
    start = clock.now();

    for(size_t i = 0; i < numOperations; i++)
    {
        if(rand() % 5 == 0 && !ptrs.empty())
        {
            free(ptrs.front());
            ptrs.pop();
            continue;
        }

        int r = rand() % 6;
        try
        {        
            void *p = malloc(allocationSizes[r]);
            ptrs.push(p);
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
    }

    end = clock.now();

    std::cout << "malloc/free : " << numOperations << " in " << duration(start, end) / 1000000.0 << " s" << '\n';    
}


void benchmarkPool(size_t totalMemory, size_t nodeSize) {
  
    size_t numOperations = 10000000;
    std::queue<void*> ptrs;

    srand(time(nullptr));

    // test stack allocator
    PoolAllocator poolAlloc(totalMemory, nodeSize);

    Clock clock;    
    Time start = clock.now();

    // random calls to Allocate() and Free()
    // if out of memory do up to 10 calls to Free() to create space
    for (size_t i = 0; i < numOperations; i++)
    {
        if(rand() % 5 == 0 && !ptrs.empty())
        {
            poolAlloc.Free(ptrs.front());
            ptrs.pop();
            continue;
        }
        
        try
        {
            void *p = poolAlloc.Allocate(nodeSize);
            ptrs.push(p);
        }
        catch(const std::exception& e)
        {
            for (size_t j = 0; j < 10; j++)
            {
                if(ptrs.empty())
                {
                    break;
                }
                   
                poolAlloc.Free(ptrs.front());
                ptrs.pop();
                ++i;
            }        
        }  
    }
    

    Time end = clock.now();

    std::cout << "\nPoolAllocator : " << numOperations << " operations in " << duration(start, end) / 1000000.0 << " s" << '\n';

    while(!ptrs.empty())
    {
        ptrs.pop();
    }
    
    // test malloc
    // random calls to malloc() and free()
    start = clock.now();

    for(size_t i = 0; i < numOperations; i++)
    {
        if(rand() % 5 == 0 && !ptrs.empty())
        {
            free(ptrs.front());
            ptrs.pop();
            continue;
        }
        
        try
        {        
            void *p = malloc(nodeSize);
            ptrs.push(p);
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
    }

    end = clock.now();

    std::cout << "malloc/free : " << numOperations << " in " << duration(start, end) / 1000000.0 << " s" << '\n';    
}


int main(int argc, char *argv[]) {
 
    uint32_t KB = 1024;
    uint32_t MB = KB*KB;

    // benchmarkStack(100*MB);
    // benchmarkList(100*MB);
    // benchmarkPool(100*MB, 1*KB);

    FreeTreeAllocator treeAlloc(10*MB);

    return 0;
}
