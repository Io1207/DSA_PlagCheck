#include <array>
#include <iostream>
#include <span>
#include <vector>
#include <cmath>
// -----------------------------------------------------------------------------
#include <unordered_map>

// You are free to add any STL includes above this comment, below the --line--.
// DO NOT add "using namespace std;" or include any other files/libraries.
// Also DO NOT add the include "bits/stdc++.h"

// OPTIONAL: Add your helper functions and data structures here

const int base = 257;
const int mod = 1e9 + 7;

std::unordered_map<long long, std::vector<int>> computeSubstringHashes(const std::vector<int> &vec, const int &window)
{
    const int n = vec.size();
    std::unordered_map<long long, std::vector<int>> hashes;

    long long hash = 0;
    long long baseExp = 1; // for removing the first element

    // Compute the hash of the first window
    for (int i = 0; i < window; i++)
    {
        hash = ((hash * base) + vec[i]) % mod;
        baseExp = (baseExp * base) % mod;
    }

    // Compute rolling hashes for all windows
    for (int i = window; i < n; ++i)
    {
        hashes[hash].push_back(i - window);
        hash = ((hash * base) + vec[i]) % mod;
        hash = (hash - ((vec[i - window] * baseExp) % mod) + mod) % mod;
    }

    // Add the last hash
    hashes[hash].push_back(n - window);

    return hashes;
}

int extendExactMatch(const std::vector<int> &submission1, const std::vector<int> &submission2, int i, int j, const int &blockSize)
{
    const int m = submission1.size();
    const int n = submission2.size();

    int i1 = i + blockSize;
    int j1 = j + blockSize;

    int matchingCount = blockSize;
    int clashingCount = 0;

    while(i1 < m && j1 < n) {
        for(int k = 0; k < blockSize && i1 + k < m && j1+ k < n; k++) {
            if(submission1[i1 + k] == submission2[j1 + k]) {
                matchingCount++;
            } else {
                clashingCount++;
            }
        }
        
        i1 += blockSize;
        j1 += blockSize;

        if(clashingCount > 0.25 * matchingCount) {
            break;
        }
    }

    while(i >= 0 && j >= 0) {
        for(int k = 0; k < blockSize && i - k >= 0 && j - k >= 0; k++) {
            if(submission1[i - k] == submission2[j - k]) {
                matchingCount++;
            } else {
                clashingCount++;
            }
        }
        
        i -= blockSize;
        j -= blockSize;

        if(clashingCount > 0.25 * matchingCount) {
            break;
        }
    }

    return i1 - i;
}

std::array<int, 5> match_submissions(std::vector<int> &submission1, std::vector<int> &submission2)
{
    // TODO: Write your code here
    const int m = submission1.size();

    auto hashes1 = computeSubstringHashes(submission1, 5);
    auto hashes2 = computeSubstringHashes(submission2, 5);

    int longestApproximateMatch = 0;
    // std::vector<bool> checked(m, false);

    for (const auto &[hash, starts1] : hashes1)
    {
        if (hashes2.find(hash) == hashes2.end())
        {
            continue;
        }
        for (const auto &i : starts1)
        {
            for (const auto &j : hashes2[hash])
            {
                longestApproximateMatch = std::max(longestApproximateMatch, extendExactMatch(submission1, submission2, i, j, 5));
            }
        }
    }

    std::array<int, 5> result = {0, 0, 0, 0, 0};

    result[2] = longestApproximateMatch;

    return result;

    // End TODO
}