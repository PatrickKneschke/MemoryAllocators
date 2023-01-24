#pragma once


#include "algorithm"
#include "allocator.h"




/* @brief Free tree implementation of IAllocator.
 * 
 * Keeps track of unallocated memory regions with a binary search tree using the start address of the free region as a key.
 * TreeNodes hold the size of the free region and the maximum size of any region in its subtree.
 * Allocates new memory from the smallest region large enough for the allocation.
 * Frees memory by creating a new TreeNode in place of the allocated memory section or merges it with direct neighbors.
 * Clears all allocations by creating a new pRoot TreeNode holding all the managed memory.
 * 
 * @class 
 */
class FreeTreeAllocator : public IAllocator{

    struct TreeNode {

        size_t size;
        size_t maxSize;
        TreeNode *parent;
        TreeNode *left;
        TreeNode *right;

        TreeNode() : size {0}, maxSize {0}, parent {nullptr}, left {nullptr}, right {nullptr} {}
        TreeNode(const size_t size_, TreeNode *parent_ = nullptr, TreeNode *left_ = nullptr, TreeNode *right_ = nullptr) : 
            size {size_}, parent {parent_}, left {left_}, right {right_} 
        {
            maxSize = size;
            if (left)
            {
                maxSize = std::max(maxSize, left->maxSize);
            }
            if (right)
            {
                maxSize = std::max(maxSize, right->maxSize);
            }
        }
    };

    struct AllocHeader {

        size_t size;
        size_t adjustment;

        AllocHeader(const size_t size_, const size_t adjustment_) : size {size_}, adjustment {adjustment} {}
    };

public:

    FreeTreeAllocator() = delete;

    /* @brief Constructor that allocates the managed memory portion and calls Clear() to reset the free tree.
     *
     * @param totalMemory    The size of the managed memory space in bytes.
     * @param parent    Optional parent allocator to get memory from.
     */
    explicit FreeTreeAllocator(const size_t totalMemory, IAllocator *parent = nullptr);

    /* @brief Default destructor that does nothing.
     */
    ~FreeTreeAllocator();
    
    /* @brief Allocates a properly aligned section of memory from the smallest TreeNode large enough for the allocation. 
     *  
     * @param size    The size of the allocated memory section.
     * @param align    The alignment of the allocated memory section. Must be non-zero and a power of two.
     * 
     * return Pointer to the allocated memory.
     */
    void* Allocate(const size_t size, const size_t align = 1) override;

    /* @brief Frees the allocated memory section at ptr and creates a new TreeNode at that position or merges the new node with direct neighbors.
     * 
     * @param ptr    Pointer to the memory position to free.
     */
    void  Free(void* ptr) override;

    /* @brief Frees all the allocated memory by creating a new pRoot TreeNode containing the whole memory.
     */
    void  Clear() override;

    /* @brief Draws a representation of the tree to console output, showing the size and maxSize of each node.
     */
    void PrintTree();


private:

    /* @brief Finds the smallest free region larger than size bytes.
     *
     * @param size    Required size of the free memory region in bytes
     * @param root    Root of the (sub-)tree to search for the memory region
     * 
     * @return Pointer to the TreeNode representing the memory region.
     */
    TreeNode* FindNode(const size_t size, TreeNode *root);

    /* @brief Inserts a new node into the tree.
     *
     * @param newNode    Pointer to the node to insert into the tree.
     */
    void InsertNode(TreeNode *newNode);

    /* @brief Removes a new node from the tree.
     *
     * @param node    Pointer to the node to be removed from the tree.
     */
    void RemoveNode(TreeNode *node);

    /* @brief Moves a node to replace another node higher up the tree.
     *
     * @param target    Pointer to the node to be replaces.
     * @param node    Pointer to the node to be oved up.
     */
    void ShiftNodeUp(TreeNode *target, TreeNode *node);

    /* @brief Updates the maxSize of a node and its parents up to the root.
     *
     * @param node    Pointer to the node to start the update at.
     */
    void UpdateMaxSize(TreeNode *node);

    /* @brief Searches the tree for the direct neighbors of the given node.
     *
     * @param node    Pointers to the left and right neighbors of the node.
     */
    std::pair<TreeNode*, TreeNode*> FindNeighbors(TreeNode *node);

    TreeNode* pRoot;
};