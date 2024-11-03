#include <array>
#include <iostream>
#include <span>
#include <vector>
#include <cmath>
// -----------------------------------------------------------------------------
#include <algorithm>
#include <unordered_map>

// You are free to add any STL includes above this comment, below the --line--.
// DO NOT add "using namespace std;" or include any other files/libraries.
// Also DO NOT add the include "bits/stdc++.h"

// OPTIONAL: Add your helper functions and data structures here

const int base = 257;
const int mod = 1e9 + 7;

std::unordered_map<long long, std::vector<int>> computeSubstringHashes(const std::vector<int> &vec, const int &blockSize)
{
    const int n = vec.size();
    std::unordered_map<long long, std::vector<int>> hashes;

    long long hash = 0;
    long long baseExp = 1; // for removing the first element

    // Compute the hash of the first blockSize
    for (int i = 0; i < blockSize; i++)
    {
        hash = ((hash * base) + vec[i]) % mod;
        baseExp = (baseExp * base) % mod;
    }

    // Compute rolling hashes for all blockSizes
    for (int i = blockSize; i < n; ++i)
    {
        hashes[hash].push_back(i - blockSize);
        hash = ((hash * base) + vec[i]) % mod;
        hash = (hash - ((vec[i - blockSize] * baseExp) % mod) + mod) % mod;
    }

    // Add the last hash
    hashes[hash].push_back(n - blockSize);

    return hashes;
}

int extendExactMatch(const std::vector<int> &submission1, const std::vector<int> &submission2, int &i, int &j, const int &blockSize)
{
    const int m = submission1.size();
    const int n = submission2.size();

    const double tolerance = 0.20;

    int i1 = i + blockSize;
    int j1 = j + blockSize;

    int matchingCount = blockSize;
    int clashingCount = 0;

    for(; i > 0 && i1 < m && j > 0 && j1 < n && clashingCount <= tolerance * matchingCount; i--, i1++, j--, j1++) {
        if(submission1[i - 1] == submission2[j - 1]) {
            matchingCount++;
        } else {
            clashingCount++;
        }
        
        if(submission1[i1] == submission2[j1]) {
            matchingCount++;
        } else {
            clashingCount++;
        }
    }
    
    return i1 - i;
}

std::ostream &operator<<(std::ostream &os, const std::vector<int> &vec)
{
    os << "[";
    for (const auto &el : vec)
    {
        os << el << ", ";
    }
    os << "]";
    return os;
}

std::array<int, 5> match_submissions(std::vector<int> &submission1, std::vector<int> &submission2)
{
    // TODO: Write your code here
    const int m = submission1.size();

    const int blockSize = 15;

    auto hashes1 = computeSubstringHashes(submission1, blockSize);
    auto hashes2 = computeSubstringHashes(submission2, blockSize);

    int longestApproximateMatch = 0;
    int start1 = 0;
    int start2 = 0;
    // std::vector<bool> checked(m, false);

    for (const auto &[hash, starts1] : hashes1)
    {
        if (hashes2.find(hash) == hashes2.end())
        {
            continue;
        }
        for (auto i : starts1)
        {
            for (auto j : hashes2[hash])
            {
                int currentApproximateMatch = extendExactMatch(submission1, submission2, i, j, blockSize);
                if (currentApproximateMatch > longestApproximateMatch)
                {
                    longestApproximateMatch = currentApproximateMatch;
                    start1 = i;
                    start2 = j;
                }
            }
        }
    }

    std::array<int, 5> result = {0, 0, 0, 0, 0};

    result[2] = longestApproximateMatch;
    result[3] = start1;
    result[4] = start2;

    return result;

    // End TODO
}