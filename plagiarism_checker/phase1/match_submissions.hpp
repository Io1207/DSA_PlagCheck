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

// node contains (col, prev, row) values
// it will be used in the AVL tree

// Define a base and a modulus for the hash function
const int base = 257;
const int mod = 1e9 + 7;

std::unordered_map<long long, std::vector<int>> getRollingHashes(const std::vector<int> &vec, const int &window) {
    std::unordered_map<long long, std::vector<int>> hashMap;
    const int n = vec.size();

    // edge case: window size is greater than the size of the vector
    if (window > n) {
        return hashMap;
    }
    
    // Compute initial hash for the first window
    long long hash = 0;
    long long basePow = 1;  // for removing the first element

    for (int i = 0; i < window; i++) {
        hash = ((hash * base) + vec[i]) % mod;
        basePow = (basePow * base) % mod;
    }

    // Compute rolling hashes for all windows
    for (int i = window; i < n; ++i) {
        hashMap[hash].push_back(i - window);
        hash = ((hash * base) + vec[i]) % mod;
        hash = (hash - ((vec[i - window] * basePow) % mod) + mod) % mod;
    }

    // Add the last window
    hashMap[hash].push_back(n - window);

    return hashMap;
}

int totalLengthOfPatternMatches(const std::vector<int> &submission1, const std::vector<int> &submission2) {
    const int minLength = 10;
    const int maxLength = 20;

    std::vector<bool> counted1(submission1.size(), false);

    for(int window = minLength; window <= maxLength; window++) {
        std::unordered_map<long long, std::vector<int>> hashMap1 = getRollingHashes(submission1, window);
        std::unordered_map<long long, std::vector<int>> hashMap2 = getRollingHashes(submission2, window);

        for(const auto &[hash, indices]: hashMap1) {
            if(hashMap2.find(hash) == hashMap2.end()) {
                continue;
            }
            for(const auto &start: indices) {
                for(int i = start; i < start + window; i++) {
                    counted1[i] = true;
                }
            }
        }
    }

    int totalLength = 0;
    for(const auto &counted: counted1) {
        if(counted) {
            totalLength++;
        }
    }
    return totalLength;
}

std::array<int, 5> match_submissions(std::vector<int> &submission1, std::vector<int> &submission2)
{
    // TODO: Write your code here

    std::array<int, 5> result{0, 0, 0, 0, 0};
        
    // std::cout << "getRollingHashes(submission1, 10) = " << getRollingHashes(submission1, 10) << std::endl;
    // std::cout << "getRollingHashes(submission2, 10) = " << getRollingHashes(submission2, 10) << std::endl;
    result[1] = totalLengthOfPatternMatches(submission1, submission2);

    return result;

    // End TODO
}