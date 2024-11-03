#include <array>
#include <iostream>
#include <span>
#include <vector>
#include <cmath>
// -----------------------------------------------------------------------------
#include <unordered_set>

// You are free to add any STL includes above this comment, below the --line--.
// DO NOT add "using namespace std;" or include any other files/libraries.
// Also DO NOT add the include "bits/stdc++.h"

// OPTIONAL: Add your helper functions and data structures here

// Define a base and a modulus for the hash function
const int base = 257;
const int mod = 1e9 + 7;

std::vector<long long> getIndexedHashes(const std::vector<int> &vec, const int &window) {
    const int n = vec.size();
    std::vector<long long> hashes(n - window + 1);

    long long hash = 0;
    long long baseExp = 1;  // for removing the first element

    // Compute the hash of the first window
    for (int i = 0; i < window; i++) {
        hash = ((hash * base) + vec[i]) % mod;
        baseExp = (baseExp * base) % mod;
    }

    // Compute rolling hashes for all windows
    for (int i = window; i < n; ++i) {
        hashes[i - window] = hash;
        hash = ((hash * base) + vec[i]) % mod;
        hash = (hash - ((vec[i - window] * baseExp) % mod) + mod) % mod;
    }

    // Add the last hash
    hashes[n - window] = hash;

    return hashes;
}

std::unordered_set<long long> getAllHashes(const std::vector<int> &vec, const int &window) {
    const int n = vec.size();
    std::unordered_set<long long> hashes;

    long long hash = 0;
    long long baseExp = 1;  // for removing the first element

    // Compute the hash of the first window
    for (int i = 0; i < window; i++) {
        hash = ((hash * base) + vec[i]) % mod;
        baseExp = (baseExp * base) % mod;
    }

    // Compute rolling hashes for all windows
    for (int i = window; i < n; ++i) {
        hashes.insert(hash);
        hash = ((hash * base) + vec[i]) % mod;
        hash = (hash - ((vec[i - window] * baseExp) % mod) + mod) % mod;
    }

    // Add the last hash
    hashes.insert(hash);

    return hashes;
}

int totalLengthOfPatternMatches(const std::vector<int> &submission1, const std::vector<int> &submission2, const int &minLength) {
    const int m = submission1.size();

    // make hashAt1, a vector of hashes of size m - 9
    // hashAt1[i] = computeHash(submission1[i ... i + 9])
    auto hashAt1 = getIndexedHashes(submission1, minLength);  

    // make hashes2, a set of all the hashes of size 10 in submission2
    auto hashes2 = getAllHashes(submission2, minLength);

    // make counted1, a vector of booleans of size m, all set to false initially
    // counted1[i] is set to true if submmission1[i] is part of a match
    std::vector<bool> counted1(m, false);

    // for each hash in hashAt1, check if it is in hashes2
    // if it is, set all the corresponding values in counted1 to true
    for(int i = 0; i < m - minLength + 1; i++) {
        if(counted1[i]) {
            continue;
        }
        if(hashes2.find(hashAt1[i]) != hashes2.end()) {
            for(int i1 = i; i1 < i + minLength; i1++) {
                counted1[i1] = true;
            }
        }
    }

    // return the total number of 'true's in counted1
    int totalLength = 0;
    for(int i = 0; i < m; i++) {
        if(counted1[i]) {
            totalLength++;
        }
    }
    return totalLength - minLength;
}

std::array<int, 5> match_submissions(std::vector<int> &submission1, std::vector<int> &submission2)
{
    // TODO: Write your code here

    std::array<int, 5> result{0, 0, 0, 0, 0};

    result[1] = totalLengthOfPatternMatches(submission1, submission2, 10);

    return result;

    // End TODO
}