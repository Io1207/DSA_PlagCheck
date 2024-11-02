#include <array>
#include <iostream>
#include <span>
#include <vector>
#include <cmath>
// -----------------------------------------------------------------------------
#include <algorithm>
#include <map>

// You are free to add any STL includes above this comment, below the --line--.
// DO NOT add "using namespace std;" or include any other files/libraries.
// Also DO NOT add the include "bits/stdc++.h"

// OPTIONAL: Add your helper functions and data structures here

// node contains (col, prev, row) values
// it will be used in the AVL tree
struct Node
{
    int col;
    int prev;
    int row;
    Node *left;
    Node *right;
    // height will be used in the AVL Tree implementation
    int height;

    Node(const int &col, const int &prev, const int &row) : col(col), prev(prev), row(row), left(nullptr), right(nullptr), height(1) {}
};

// implementation of AVL tree
// it acts as a binary search tree using 'col' value
// to be used as a map in the GridSearch class
class AVLTree
{
public:
    AVLTree() : root(nullptr) {}

    ~AVLTree()
    {
        recursiveDestructor(root);
    }

protected:
    Node *root;

    void recursiveDestructor(Node *node)
    {
        if (!node)
        {
            return;
        }
        recursiveDestructor(node->left);
        recursiveDestructor(node->right);
        delete node;
    }

    // helper function to get the height of a node
    int getHeight(Node *node) const
    {
        if (!node)
        {
            return 0;
        }
        return node->height;
    }

    // helper function to set the height of a node
    void setHeight(Node *node)
    {
        node->height = 1 + std::max(getHeight(node->left), getHeight(node->right));
    }

    // helper function to get the balance factor of a node
    // balanceFactor = height of left subtree - height of right subtree
    // balanceFactor belongs to {-1, 0, 1} for a balanced tree
    int getBalanceFactor(Node *node) const
    {
        if (!node)
        {
            return 0;
        }
        return getHeight(node->left) - getHeight(node->right);
    }

    // rotation functions
    Node *rotateLL(Node *curr)
    {
        // perform the rotation
        Node *next = curr->left;
        curr->left = next->right;
        next->right = curr;

        // update the heights
        setHeight(curr);
        setHeight(next);

        // return the new root
        return next;
    }

    Node *rotateRR(Node *curr)
    {
        // same as rotateLL, simply interchange left and right

        Node *next = curr->right;
        curr->right = next->left;
        next->left = curr;

        setHeight(curr);
        setHeight(next);

        return next;
    }

    Node *rotateLR(Node *curr)
    {
        // perform the rotation
        Node *next = curr->left->right;
        curr->left->right = next->left;
        next->left = curr->left;
        curr->left = next->right;
        next->right = curr;

        // update the heights
        setHeight(curr);
        setHeight(next->left);
        setHeight(next);

        // return the new root
        return next;
    }

    Node *rotateRL(Node *curr)
    {
        // simply interchange left and right in rotateLR

        Node *next = curr->right->left;
        curr->right->left = next->right;
        next->right = curr->right;
        curr->right = next->left;
        next->left = curr;

        setHeight(curr);
        setHeight(next->right);
        setHeight(next);

        return next;
    }

    // function to balance the subtree rooted at curr
    // returns the new root of the subtree
    Node *balanceSubtree(Node *curr)
    {
        int balanceFactor = getBalanceFactor(curr);

        // check the 4 cases of imbalance
        if (balanceFactor > 1)
        {
            if (getBalanceFactor(curr->left) >= 0)
            {
                return rotateLL(curr);
            }
            return rotateLR(curr);
        }
        if (balanceFactor < -1)
        {
            if (getBalanceFactor(curr->right) >= 0)
            {
                return rotateRL(curr);
            }
            return rotateRR(curr);
        }

        // if balanced, then the root of the subtree remains the same
        return curr;
    }

    // helper function for recursive insertion
    Node *insertInSubtree(Node *curr, Node *ptr)
    {
        // base case
        if (!curr)
        {
            return ptr;
        }

        // it is guaranteed that col is unique, so no need to check for equality
        if (ptr->col < curr->col)
        {
            curr->left = insertInSubtree(curr->left, ptr);
        }
        else
        {
            curr->right = insertInSubtree(curr->right, ptr);
        }

        // re-balance the tree and return it
        return balanceSubtree(curr);
    }
};

// implementation of GridSearch
// it uses an AVL tree to store the nodes
// it also stores the nodes in a vector for quick access
class GridSearch : public AVLTree
{
    std::vector<Node *> nodeOf;

    // helper function for recursive deletion
    Node *eraseFromSubtree(Node *curr, Node *ptr)
    {
        // base case
        if (!curr)
        {
            return nullptr;
        }

        // search for the node to be deleted
        if (ptr->col < curr->col)
        {
            curr->left = eraseFromSubtree(curr->left, ptr);
        }
        else if (ptr->col > curr->col)
        {
            curr->right = eraseFromSubtree(curr->right, ptr);
        }
        else
        {
            // node found
            if (!curr->left || !curr->right)
            {
                // node has at most one child
                Node *temp = curr->left ? curr->left : curr->right;
                delete curr;
                return temp;
            }

            // node has two children
            // find the inorder successor
            Node *temp = curr->right;
            while (temp->left)
            {
                temp = temp->left;
            }

            // copy the data of the inorder successor
            curr->col = temp->col;
            curr->prev = temp->prev;
            curr->row = temp->row;
            nodeOf[curr->prev] = curr;

            // delete the inorder successor
            nodeOf[temp->prev] = nullptr;
            curr->right = eraseFromSubtree(curr->right, temp);
        }

        // re-balance the tree and return it
        return balanceSubtree(curr);
    }

public:
    GridSearch(const int &maxLength) : AVLTree(), nodeOf(1 + maxLength, nullptr)
    {
        std::cout << "maxLength = " << maxLength << std::endl;
    }

    void insert(const int &col, const int &prev, const int &row)
    {
        Node *ptr = new Node(col, prev, row);
        nodeOf[prev] = ptr;
        root = insertInSubtree(root, ptr);
    }

    void erase(Node *ptr)
    {
        nodeOf[ptr->prev] = nullptr;
        root = eraseFromSubtree(root, ptr);
    }

    int closestUpto(const int &col, std::pair<int, int> &parent)
    {
        Node *currNode = root;
        Node *prevNode = nullptr;

        while (currNode)
        {
            if (currNode->col == col)
            {
                parent = {currNode->row, currNode->col};
                return currNode->prev;
            }
            if (currNode->col > col)
            {
                currNode = currNode->left;
            }
            else
            {
                prevNode = currNode;
                currNode = currNode->right;
            }
        }

        parent = {prevNode->row, prevNode->col};
        return prevNode->prev;
    }

    void reduceStart(const int &col, const int &prev, const int &row)
    {
        Node *ptr = nodeOf[prev];
        if (!ptr)
        {
            insert(col, prev, row);
        }
        else if (col < ptr->col)
        {
            erase(ptr);
            insert(col, prev, row);
        }
    }
};

int totalLengthOfPatternMatches(
    const std::vector<int> &submission1,
    const std::vector<int> &submission2,
    const int &minLength = 10,
    const int &maxLength = 20)
{
    const int m = submission1.size();
    const int n = submission2.size();

    // initialize the dp and parentOf tables
    // dp[i][j] = length of longest pattern match ending at i - 1 in submission1 and j - 1 in submission2
    // parentOf[i][j] = (i0, j0) such that dp[i][j] = dp[i0][j0] + 1
    std::vector<std::vector<int>> dp(m + 1, std::vector<int>(n + 1, 0));
    std::vector<std::vector<std::pair<int, int>>> parentOf(m + 1, std::vector<std::pair<int, int>>(n + 1, {-1, -1}));

    // initialize the grid searcher
    // it will be used to find the closest pattern match ending at j - 1
    GridSearch gs(std::min(m, n));
    // insert the node (0, 0, 0) by default
    gs.insert(0, 0, 0);

    // populate the dp and parentOf tables
    for (int i = 1; i <= m; i++)
    {
        for (int j = 1; j <= n; j++)
        {
            if (submission1[i - 1] == submission2[j - 1])
            {
                // extend the pattern match
                dp[i][j] = 1 + gs.closestUpto(j - 1, parentOf[i][j]);
            }
        }

        // prepare gs for the next row
        for (int j = 1; j <= n; j++)
        {
            gs.reduceStart(j, dp[i][j], i);
        }
    }

    // initialize and populate the final occurrence map for submission1 and submission2
    std::map<int, int> finalOccurrence1;
    std::map<int, int> finalOccurrence2;

    for (int i = 0; i < m; i++)
    {
        finalOccurrence1[submission1[i]] = i;
    }

    for (int j = 0; j < n; j++)
    {
        finalOccurrence2[submission2[j]] = j;
    }

    // initialize the boolean vector to track the counted elements of submission1
    // // not required for submission2 as the counted elements will be the same
    std::vector<bool> counted1(m, false);
    std::vector<bool> counted2(n, false);

    for (auto [token, i] : finalOccurrence1)
    {
        if (finalOccurrence2.find(token) == finalOccurrence2.end())
        {
            continue;
        }
        int j = finalOccurrence2[token];
        if (dp[i + 1][j + 1] < minLength)
        {
            continue;
        }
        while (i >= 0 && j >= 0)
        {
            if (dp[i + 1][j + 1] <= maxLength)
            {
                counted1[i] = true;
                counted2[j] = true;
            }
            auto [i0, j0] = parentOf[i + 1][j + 1];
            i = i0 - 1;
            j = j0 - 1;
        }
    }

    // calculate and return the total length of the pattern matches
    int totalLength = 0;
    for (int i = 0; i < m; i++)
    {
        if (counted1[i])
        {
            totalLength++;
        }
    }
    for(int j = 0; j < n; j++)
    {
        if (counted2[j])
        {
            totalLength++;
        }
    }

    return totalLength;
}

std::array<int, 5> match_submissions(std::vector<int> &submission1, std::vector<int> &submission2)
{
    // TODO: Write your code here

    std::array<int, 5> result{0, 0, 0, 0, 0};
    // result[1] = totalLengthOfPatternMatches(submission1, submission2, 10, 20);
    result[1] = totalLengthOfPatternMatches(submission1, submission2, 5, 5);
    return result;

    // End TODO
}