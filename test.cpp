
#include "free_list_allocator.h"
#include "free_tree_allocator.h"
#include "pool_allocator.h"
#include "stack_allocator.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <queue>
#include <random>
#include <string>
#include <unordered_set>
#include <vector>


using Clock = std::chrono::high_resolution_clock;
using Time  = std::chrono::time_point<std::chrono::high_resolution_clock>;


int64_t duration(Time start, Time end) {

    return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}


void benchmarkStack(size_t totalMemory, size_t numOperations) {

    std::vector<size_t> allocationSizes = {16, 64, 256, 1024, 4096, 16384};
   
    auto seed =  time(nullptr);
    std::cout << seed << '\n';
    srand(seed);
    

    // test stack allocator
    StackAllocator stAlloc(totalMemory);

    Clock clock;    
    Time start = clock.now();

    // calls to Allocate() with random sizes
    // if out of memory clear stack
    for (size_t i = 0; i < numOperations; i++)
    {         
        int r = rand() % 6;
        try
        {
            void *p = stAlloc.Allocate(allocationSizes[r]);
        }
        catch(const std::exception& e)
        {
            stAlloc.Clear();          
        }        
    }

    Time end = clock.now();

    std::cout << "StackAllocator : " << numOperations << " operations in " << duration(start, end) / 1000000.0 << " s" << " , max memory " << stAlloc.maxUsedMemory() << '\n';
}


void benchmarkList(size_t totalMemory, size_t numOperations) {
  
    std::vector<size_t> allocationSizes = {16, 64, 256, 1024, 4096, 16384};
    std::unordered_set<void*> ptrs;

    auto seed = time(nullptr);
    std::cout << seed << '\n';
    srand(seed);

    // test stack allocator
    FreeListAllocator listAlloc(totalMemory);

    Clock clock;    
    Time start = clock.now();

    // calls to Allocate() and Free() with random sizes
    // if out of memory do up to 10 calls to Free() to create space
    for (size_t i = 0; i < numOperations; i++)
    {
        if(rand() % 3 == 0 && !ptrs.empty())
        {
            auto pos = std::next(ptrs.begin(), rand() % ptrs.size());
            listAlloc.Free( *(pos) );
            ptrs.erase(pos);
            continue;
        }
         
        int r = rand() % 6;
        try
        {
            void *p = listAlloc.Allocate(allocationSizes[r]);
            ptrs.insert(p);
        }
        catch(const std::exception& e)
        {
            
            for (size_t j = 0; j < 10; j++)
            {
                if(ptrs.empty())
                {
                    break;
                }

                auto pos = std::next(ptrs.begin(), rand() % ptrs.size());
                listAlloc.Free( *(pos) );
                ptrs.erase(pos);
                ++i;
            }            
        }        
    }
    
    Time end = clock.now();

    std::cout << "FreeListAllocator : " << numOperations << " operations in " << duration(start, end) / 1000000.0 << " s" << " , max memory " << listAlloc.maxUsedMemory() << '\n';

    std::cout << " used " << listAlloc.usedMemory()  << ", free " << listAlloc.totalMemory() - listAlloc.usedMemory() << '\n';
}


void benchmarkTree(size_t totalMemory, size_t numOperations) {

    std::vector<size_t> allocationSizes = {16, 64, 256, 1024, 4096, 16384};
    std::unordered_set<void*> ptrs;

    auto seed = time(nullptr);
    std::cout << seed << '\n';
    srand(seed);

    // test stack allocator
    FreeTreeAllocator treeAlloc(totalMemory);

    Clock clock;    
    Time start = clock.now();

    // calls to Allocate() and Free() with random sizes
    // if out of memory do up to 10 calls to Free() to create space
    for (size_t i = 0; i < numOperations; i++)
    {
        if(rand() % 3 == 0 && !ptrs.empty())
        {
            auto pos = std::next(ptrs.begin(), rand() % ptrs.size());
            treeAlloc.Free( *(pos) );
            ptrs.erase(pos);
            continue;
        }
         
        int r = rand() % 6;
        try
        {
            void *p = treeAlloc.Allocate(allocationSizes[r]);
            ptrs.insert(p);
        }
        catch(const std::exception& e)
        {
            
            for (size_t j = 0; j < 10; j++)
            {
                if(ptrs.empty())
                {
                    break;
                }

                auto pos = std::next(ptrs.begin(), rand() % ptrs.size());
                treeAlloc.Free( *(pos) );
                ptrs.erase(pos);
                ++i;
            }            
        }        
    }

    Time end = clock.now();

    std::cout << "FreeTreeAllocator : " << numOperations << " operations in " << duration(start, end) / 1000000.0 << " s" << " , max memory " << treeAlloc.maxUsedMemory() << '\n';

    std::cout << " used " << treeAlloc.usedMemory()  << ", free " << treeAlloc.totalMemory() - treeAlloc.usedMemory() << '\n';
}


void benchmarkPool(size_t totalMemory, size_t nodeSize, size_t numOperations) {
  
    std::unordered_set<void*> ptrs;

    auto seed = time(nullptr);
    std::cout << seed << '\n';
    srand(seed);

    // test stack allocator
    PoolAllocator poolAlloc(totalMemory, nodeSize);

    Clock clock;    
    Time start = clock.now();

    // calls to Allocate() and Free() with random sizes
    // if out of memory do up to 10 calls to Free() to create space
    for (size_t i = 0; i < numOperations; i++)
    {
        if(rand() % 3 == 0 && !ptrs.empty())
        {
            auto pos = std::next(ptrs.begin(), rand() % ptrs.size());
            poolAlloc.Free( *(pos) );
            ptrs.erase(pos);
            continue;
        }
         
        int r = rand() % 6;
        try
        {
            void *p = poolAlloc.Allocate(nodeSize);
            ptrs.insert(p);
        }
        catch(const std::exception& e)
        {
            
            for (size_t j = 0; j < 10; j++)
            {
                if(ptrs.empty())
                {
                    break;
                }

                auto pos = std::next(ptrs.begin(), rand() % ptrs.size());
                poolAlloc.Free( *(pos) );
                ptrs.erase(pos);
                ++i;
            }            
        }        
    }

    Time end = clock.now();

    std::cout << "PoolAllocator : " << numOperations << " operations in " << duration(start, end) / 1000000.0 << " s" << " , max memory " << poolAlloc.maxUsedMemory() << '\n'; 
}


void benchmarkMalloc(size_t numOperations) {

    std::vector<size_t> allocationSizes = {16, 64, 256, 1024, 4096, 16384};
    std::queue<void*> ptrs;

    auto seed =  time(nullptr);
    std::cout << seed << '\n';
    srand(seed);

    Clock clock;    
    Time start = clock.now(), end;
    
    // calls to malloc() and free() with random sizes
    start = clock.now();

    for(size_t i = 0; i < numOperations; i++)
    {
        if(rand() % 3 == 0 && !ptrs.empty())
        {
            free(ptrs.front());
            ptrs.pop();
            continue;
        }

        int r = rand() % 6;
        void *p = malloc(allocationSizes[r]);
        ptrs.push(p);
    }

    end = clock.now();

    std::cout << "malloc/free : " << numOperations << " in " << duration(start, end) / 1000000.0 << " s" << '\n';   
}


int main(int argc, char *argv[]) {
 
    uint32_t KB = 1024;
    uint32_t MB = KB*KB;

    benchmarkMalloc(1000000);
    benchmarkStack(10*MB, 1000000);
    benchmarkList(10*MB, 1000000);
    benchmarkTree(10*MB, 1000000);
    benchmarkPool(10*MB, 1*KB, 1000000);


    return 0;
}
