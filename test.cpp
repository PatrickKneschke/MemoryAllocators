
#include "free_list_allocator.h"
#include "stack_allocator.h"

#include <chrono>
#include <iostream>
#include <string>
#include <vector>


using Clock = std::chrono::high_resolution_clock;
using Time  = std::chrono::time_point<std::chrono::high_resolution_clock>;


int64_t duration(Time start, Time end) {

    return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}


void benchmarkStack(size_t totalMemory) {

    std::vector<size_t> allocationSizes = {32, 64, 128, 256, 512, 1024};
    std::vector<size_t> numAllocations = {totalMemory/32/2, totalMemory/64, totalMemory/128, totalMemory/256, totalMemory/512, totalMemory/1024};
   
    size_t numOperations = 0;
    for(size_t i = 0; i < numAllocations.size(); i++)
    {
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


int main(int argc, char *argv[]) {
 
    uint32_t KB = 1024;
    uint32_t MB = KB*KB;

    //benchmarkStack(10*MB);
    FreeListAllocator listAlloc(10*KB);
    
    void *p1 = listAlloc.Allocate(64, 16);
    void *p2 = listAlloc.Allocate(64, 16);
    void *p3 = listAlloc.Allocate(64, 16);
    listAlloc.Free(p2);
    listAlloc.Free(p1);

    return 0;
}
