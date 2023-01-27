
#include "free_tree_allocator.h"
#include <stdexcept>


// JUST FOR TESTING

#include <iostream>

void FreeTreeAllocator::PrintTree() {

    std::function<void(std::string, TreeNode*, bool)> printTree;
    printTree = [&printTree](std::string prefix, TreeNode *root, bool isLeft) {

        if (root == nullptr)
        {
            return;
        }

        std::cout << prefix;
        std::cout << (isLeft ? "├──" : "└──" );
        std::cout << root->size << ":" << root->maxSize << '\n';
        
        printTree(prefix + (isLeft ? "│   " : "    "), root->left, true);
        printTree(prefix + (isLeft ? "│   " : "    "), root->right, false);
    
    };

    printTree("", pRoot, false);

    std::cout << "----------------------------\n";
}

// JUST FOR TESTING END


FreeTreeAllocator::FreeTreeAllocator(const size_t totalMemory, IAllocator *parent) :
    IAllocator(totalMemory, parent),
    pRoot {nullptr}
{
    Clear();
}

FreeTreeAllocator::~FreeTreeAllocator() {

}

void* FreeTreeAllocator::Allocate(const size_t size, const size_t align) {

    // Pad size so that total allocated space can fit a TreeNode when freed
    size_t paddedSize = std::max(size, sizeof(TreeNode) - sizeof(AllocHeader));

    // Find best memory region to allocate from
    size_t requiredSize = paddedSize + sizeof(AllocHeader) + align - 1;    
    TreeNode *allocNode = FindNode(requiredSize, pRoot);
    if (allocNode == nullptr)
    {
        throw std::overflow_error("Free tree allocator does not have a large enough memory region available.");
    }

    // Get aligned address for allocation
    size_t adjustment = getAlignmentAdjustment(reinterpret_cast<uintptr_t>(allocNode) + sizeof(AllocHeader), align);
    uintptr_t alignedAddress = reinterpret_cast<uintptr_t>(allocNode) + adjustment + sizeof(AllocHeader);

    // Create new node for remaining memory region if it is large enough to fit a TreeNode
    // If not use the whole free region for allocation
    size_t newSize = reinterpret_cast<uintptr_t>(allocNode) + allocNode->size - alignedAddress - paddedSize;
    size_t allocSize = paddedSize;
    if (newSize >= sizeof(TreeNode))
    {
        TreeNode *newNode = new (reinterpret_cast<void*>(alignedAddress + paddedSize)) TreeNode(newSize);
        ReplaceNode(allocNode, newNode);
    }
    else
    {
        allocSize += newSize;
        RemoveNode(allocNode);
    }

    // Place allocation header in front of allocated memory section
    AllocHeader *header = reinterpret_cast<AllocHeader*>(alignedAddress - sizeof(AllocHeader));
    header->size = allocSize;
    header->adjustment = adjustment;

    mUsedMemory += header->adjustment + sizeof(AllocHeader) + header->size;
    mMaxUsedMemory = std::max(mMaxUsedMemory, mUsedMemory);

    return reinterpret_cast<void*>(alignedAddress);
}

void FreeTreeAllocator::Free(void* ptr) {

    assert(ptr != nullptr);

    // start address and size of freed memory section
    uintptr_t freeAddress = reinterpret_cast<uintptr_t>(ptr);
    AllocHeader *header = reinterpret_cast<AllocHeader*>( freeAddress - sizeof(AllocHeader) );
    freeAddress -= header->adjustment + sizeof(AllocHeader);
    size_t freeSize = header->adjustment + sizeof(AllocHeader) + header->size;

    mUsedMemory -= freeSize;
    
    TreeNode *newNode = new (reinterpret_cast<void*>(freeAddress)) TreeNode(freeSize);

    // try merge adjacent nodes
    auto [leftNode, rightNode] = FindNeighbors(newNode);
    if (rightNode && reinterpret_cast<uintptr_t>(newNode) + newNode->size == reinterpret_cast<uintptr_t>(rightNode))
    {
        newNode->size += rightNode->size;
        RemoveNode(rightNode); 
    }
    if (leftNode && reinterpret_cast<uintptr_t>(leftNode) + leftNode->size == reinterpret_cast<uintptr_t>(newNode))
    {
        leftNode->size += newNode->size;
        UpdateMaxSize(leftNode);
    }
    else
    {
        InsertNode(newNode);
        UpdateMaxSize(newNode);
    }
}

void FreeTreeAllocator::Clear() {
    
    pRoot = new (pBase) TreeNode(mTotalMemory);
    mUsedMemory = 0;
}

FreeTreeAllocator::TreeNode* FreeTreeAllocator::FindNode(const size_t size, TreeNode *root) {

    if (!root || root->maxSize < size)
    {
        return nullptr;
    }

    if (root->size >= size)
    {
        return root;
    }

    if (root->left && root->left->maxSize >= size)
    {
        return FindNode(size, root->left);
    }

    return FindNode(size, root->right);

    // size_t leftMax = root->left ? root->left->maxSize : 0;
    // size_t rightMax = root->right ? root->right->maxSize : 0;
    // if (leftMax <= rightMax && leftMax >= size)
    // {
    //     return FindNode(size, root->left);
    // }
    // else if(rightMax >= size)
    // {
    //     return FindNode(size, root->right);
    // }

    // return root;
}

void FreeTreeAllocator::InsertNode(TreeNode *newNode) {

    if(!pRoot)
    {
        pRoot =  newNode;
        return;
    }

    TreeNode *curr = pRoot, *prev = nullptr;
    while (curr)
    {
        prev = curr;
        curr->maxSize = std::max(curr->maxSize, newNode->size);

        if (reinterpret_cast<uintptr_t>(newNode) < reinterpret_cast<uintptr_t>(curr))
        {
            curr = curr->left;
        }
        else
        {
            curr = curr->right;
        }
    }

    newNode->parent = prev;
    if (reinterpret_cast<uintptr_t>(newNode) < reinterpret_cast<uintptr_t>(prev))
    {
        prev->left = newNode;
    }
    else
    {
        prev->right = newNode;
    }
}

void FreeTreeAllocator::RemoveNode(TreeNode *node) {

    // Node from which to start the maxSize update
    TreeNode *sizeUpdateNode = node->parent;
    
    if (!node->left)
    {
        ShiftNodeUp(node, node->right);
    }
    else if (!node->right)
    {
        ShiftNodeUp(node, node->left);
    }
    else
    {
        TreeNode *nextNode = node->right;
        while (nextNode->left)
        {
            nextNode = nextNode->left;
        }

        if (nextNode->parent != node)
        {
            sizeUpdateNode = nextNode->parent;
            ShiftNodeUp(nextNode, nextNode->right);
            nextNode->right = node->right;
            nextNode->right->parent = nextNode;
        }
        else
        {
            sizeUpdateNode = nextNode;
        }
        ShiftNodeUp(node, nextNode);
        nextNode->left = node->left;
        nextNode->left->parent = nextNode;
    }

    UpdateMaxSize(sizeUpdateNode);
}

void FreeTreeAllocator::ReplaceNode(TreeNode *target, TreeNode *newNode) {

    if (target == pRoot)
    {
        pRoot = newNode;            
    }
    else
    {
        newNode->parent = target->parent;
        target == target->parent->left ? target->parent->left = newNode : target->parent->right = newNode;
    }

    if (target->left)
    {
        newNode->left = target->left;
        newNode->left->parent = newNode;
    }

    if (target->right)
    {
        newNode->right = target->right;
        newNode->right->parent = newNode;
    }

    UpdateMaxSize(newNode->parent);
}

void FreeTreeAllocator::ShiftNodeUp(TreeNode *target, TreeNode *node) {

    if (node != nullptr)
    {
        node->parent = target->parent;
    }

    if(target == pRoot)
    {
        pRoot = node;
    }    
    else if (target == target->parent->left)
    {
        target->parent->left = node;
    }
    else
    {
        target->parent->right = node;
    }
}

void FreeTreeAllocator::UpdateMaxSize(TreeNode *node) {

    while (node)
    {
        node->maxSize = node->size;
        if (node->left)
        {
            node->maxSize = std::max(node->maxSize, node->left->maxSize);
        }

        if (node->right)
        {
            node->maxSize = std::max(node->maxSize, node->right->maxSize);
        }

        node = node->parent;
    }
}

std::pair<FreeTreeAllocator::TreeNode*, FreeTreeAllocator::TreeNode*> FreeTreeAllocator::FindNeighbors(TreeNode *node) {

    TreeNode *curr = pRoot, *left = nullptr, *right = nullptr;
    while (curr && curr != node)
    {
        if (reinterpret_cast<uintptr_t>(node) < reinterpret_cast<uintptr_t>(curr))
        {
            right = curr;
            curr = curr->left;
        }
        else
        {
            left = curr;
            curr = curr->right;
        }
    }

    return {left, right};
}