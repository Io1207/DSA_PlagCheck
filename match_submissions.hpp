#include<array>
#include<iostream>
#include<span>
#include<vector>
#include<cmath>
#include<algorithm>
#include<unordered_set>
#include<functional>
// -----------------------------------------------------------------------------

// You are free to add any STL includes above this comment, below the --line--.
// DO NOT add "using namespace std;" or include any other files/libraries.
// Also DO NOT add the include "bits/stdc++.h"

// OPTIONAL: Add your helper functions and data structures here

struct VectorHash
//calculates hash for the vector
//code inspired by https://stackoverflow.com/questions/10405030/c-unordered-map-fail-when-used-with-a-vector-as-key
{
    std::size_t operator()(const std::vector<int>& vec) const 
    {
        std::size_t hash = 0;
        for (int num : vec) 
        {
            hash ^= std::hash<int>{}(num) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        }
        return hash;
    }
};

struct RollingHash
{
    const int base=257;
    const int mod=10000007;

    int computeHash(const std::vector<int>& arr, int length)
    {
        int hash=0;
        for (int i=0; i<length; i++) hash=(1LL*hash*base+arr[i])%mod;
        return hash;
    }

    int rollingHash(int currHash,int length, int deleting, int adding)
    {
        current_hash=(1LL*currHash*base-1LL*deleting*power(base,length)%mod+adding)%mod;
        if (currHash<0) currHash=currHash+mod;
        return currHash;
    }

    int power(int base, int power)
    {
        int reult=1;
        while(power>0)
        {
            if (exp%2!=0) reult=(1LL*reult*base)%mod;
            base=(1LL*base*base)%mod;
            power=power/2;
        }
        return reult;
    }

};

bool is_approximate_match(const std::vector<int>& arr1, size_t start1, const std::vector<int>& arr2, size_t start2, size_t len, double similarity=0.8) 
//hecking for long approximate match 
{
    int match_count = 0;
    for (int i = 0; i < len; ++i) 
    {
        if (arr1[start1 + i] == arr2[start2 + i]) 
        {
            ++match_count;
        }
        if ((match_count + (len - i - 1)) < similarity * len) return false;
    }
    return (match_count >= similarity * len);
}

std::vector<std::tuple<int, int, int>> find_similar_subarrays(const std::vector<int>& arr1, const std::vector<int>& arr2, int min_len = 30, double similarity = 0.8) 
{
    RollingHash hasher;
    std::unordered_map<int, std::vector<int>> arr1_hashes; // Map of hash -> starting indices

    // Precompute hashes for all subarrays of length min_len in arr1
    int initial_hash1 = hasher.compute_hash(arr1, min_len);
    arr1_hashes[initial_hash1].push_back(0);
    for (int i = 1; i + min_len <= arr1.size(); ++i) 
    {
        initial_hash1 = hasher.roll_hash(initial_hash1, min_len, arr1[i - 1], arr1[i + min_len - 1]);
        arr1_hashes[initial_hash1].push_back(i);
    }

    // Find matches in arr2 using rolling hash and approximate similarity check
    std::vector<std::tuple<int, int, int>> matches; // (start1, start2, length)
    int initial_hash2 = hasher.compute_hash(arr2, min_len);
    for (const int& start1 : arr1_hashes[initial_hash2]) 
    {
        if (is_approximate_match(arr1, start1, arr2, 0, min_len, similarity)) 
        {
            int match_len = min_len;
            while (start1 + match_len < arr1.size() && match_len < arr2.size() && arr1[start1 + match_len] == arr2[match_len]) 
            {
                ++match_len;
            }
            matches.emplace_back(start1, 0, match_len);
        }
    }
    for (int j = 1; j + min_len <= arr2.size(); ++j) {
        initial_hash2 = hasher.roll_hash(initial_hash2, min_len, arr2[j - 1], arr2[j + min_len - 1]);
        if (arr1_hashes.count(initial_hash2)) 
        {
            for (const int& start1 : arr1_hashes[initial_hash2]) 
            {
                if (is_approximate_match(arr1, start1, arr2, j, min_len, similarity)) 
                {
                    int match_len = min_len;
                    while (start1 + match_len < arr1.size() && j + match_len < arr2.size() && arr1[start1 + match_len] == arr2[j + match_len]) 
                    {
                        ++match_len;
                    }
                    matches.emplace_back(start1, j, match_len);
                }
            }
        }
    }

    return matches;
}

std::array<int, 5> match_submissions(std::vector<int>& submission1, std::vector<int>& submission2) 
{
    int shortMatchLength = 0;
    int longMatchLength = 0;
    int totalMatches=0;
    int longStart1 = -1, longStart2 = -1;
    int isPlagiarized = false;
    double numberThreshold=10;
    double lengthThreshold=75;

    //Short exact matches (10-20 tokens)
    for (int len = 10; len <= 20; ++len) 
    {
        std::unordered_set<std::vector<int>,VectorHash> sub1_patterns;
        for (size_t i = 0; i + len <= submission1.size(); ++i)   sub1_patterns.insert(std::vector<int>(submission1.begin() + i, submission1.begin() + i + len));
        for (size_t j = 0; j + len <= submission2.size(); ++j) 
        {
            std::vector<int> sub2_subarray(submission2.begin() + j, submission2.begin() + j + len);
            if (sub1_patterns.count(sub2_subarray)) 
            {
                shortMatchLength += len;
                // Avoid overlaps in sub1_patterns to prevent double-counting
                sub1_patterns.erase(sub2_subarray);
            }
        }
    }

    //Long approximate matches (30+ tokens with 80% similarity)
    for (size_t len = 30; len <= submission1.size() && len <= submission2.size(); ++len) 
    {
        for (size_t i = 0; i + len <= submission1.size(); ++i) 
        {
            for (size_t j = 0; j + len <= submission2.size(); ++j) 
            {
                if (is_approximate_match(submission1, i, submission2, j, len, 0.8)) 
                {
                    if (len > longMatchLength) 
                    {
                        longMatchLength = len;
                        longStart1 = i;
                        longStart2 = j;
                    }
                }
            }
        }
    }

    isPlagiarized = (totalMatches >= numberThreshold || longMatchLength >= lengthThreshold);
    //Return Results
    return {isPlagiarized, shortMatchLength, longMatchLength, longStart1, longStart2};
}