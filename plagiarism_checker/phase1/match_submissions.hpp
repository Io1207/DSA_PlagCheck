#include <array>
#include <iostream>
#include <span>
#include <vector>
#include <cmath>
// -----------------------------------------------------------------------------
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

const int base = 257;
const int mod = 1e9 + 7;

// computing hashes for different block sizes to finally implement rolling hash to find exact matches
std::unordered_map<long long, std::vector<int>> computeSubstringHashes(const std::vector<int> &vec, const int &blockSize)
{
    const int n = vec.size();
    std::unordered_map<long long, std::vector<int>> hashes;

    long long hash = 0;
    long long baseExp = 1; // to remove the first element

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

// extending exact match on both ends to look for the approximate match
int extendExactMatch(const std::vector<int> &submission1, const std::vector<int> &submission2, int &i, int &j, const int &blockSize)
{
    const int m = submission1.size();
    const int n = submission2.size();

    const double tolerance = 0.20;

    int i1 = i + blockSize;
    int j1 = j + blockSize;

    int matchingCount = blockSize;
    int clashingCount = 0;

    for (; i > 0 && i1 < m && j > 0 && j1 < n && clashingCount <= tolerance * matchingCount; i--, i1++, j--, j1++)
    {
        if (submission1[i - 1] == submission2[j - 1])
            matchingCount++;
        else
            clashingCount++;

        if (submission1[i1] == submission2[j1])
            matchingCount++;
        else
            clashingCount++;
    }

    return i1 - i;
}

// hashing of window starting at a particular index
std::vector<long long> getIndexedHashes(const std::vector<int> &vec, const int &window)
{
    const int n = vec.size();
    std::vector<long long> hashes(n - window + 1);

    long long hash = 0;
    long long baseExp = 1; // to remove the first element

    // Compute the hash of the first window
    for (int i = 0; i < window; i++)
    {
        hash = ((hash * base) + vec[i]) % mod;
        baseExp = (baseExp * base) % mod;
    }

    // Compute rolling hashes for all windows
    for (int i = window; i < n; ++i)
    {
        hashes[i - window] = hash;
        hash = ((hash * base) + vec[i]) % mod;
        hash = (hash - ((vec[i - window] * baseExp) % mod) + mod) % mod;
    }

    // Add the last hash
    hashes[n - window] = hash;

    return hashes;
}

//Getting the set of all hashes for all windows of size 'window'
std::unordered_set<long long> getAllHashes(const std::vector<int> &vec, const int &window)
{
    const int n = vec.size();
    std::unordered_set<long long> hashes;

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
        hashes.insert(hash);
        hash = ((hash * base) + vec[i]) % mod;
        hash = (hash - ((vec[i - window] * baseExp) % mod) + mod) % mod;
    }

    // Add the last hash
    hashes.insert(hash);

    return hashes;
}

// getting the length of all exact matches
int totalLengthOfPatternMatches(const std::vector<int> &submission1, const std::vector<int> &submission2, const int &minLength)
{
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
    for (int i = 0; i < m - minLength + 1; i++)
    {
        if (counted1[i])
            continue;
        if (hashes2.find(hashAt1[i]) != hashes2.end())
        {
            for (int i1 = i; i1 < i + minLength; i1++)
            {
                counted1[i1] = true;
            }
        }
    }

    // return the total number of 'true's in counted1
    int totalLength = std::count(counted1.begin(), counted1.end(), true);
    return totalLength - minLength;
}

// finding the longest approximate matches between two files
int maximumApproximatePatternMatch(const std::vector<int> &submission1, const std::vector<int> &submission2, int &start1, int &start2)
{
    const int m = submission1.size();

    const int blockSize = 15;

    auto hashes1 = computeSubstringHashes(submission1, blockSize);
    auto hashes2 = computeSubstringHashes(submission2, blockSize);

    int longestApproximateMatch = 0;

    for (const auto &[hash, starts1] : hashes1)
    {
        if (hashes2.find(hash) == hashes2.end())
            continue;
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

    return longestApproximateMatch;
}

// flagging as plagiarised or not plagiarised
int flagged(int size, int exact, int longest)
{
    if ((float)exact / (float)size >= 0.2 || exact >= 200 || longest >= 150 || (float)longest / (float)size >= 0.2)
    {
        return 1;
    }
    return 0;
}

// main code that returns result array
std::array<int, 5> match_submissions(std::vector<int> &submission1, std::vector<int> &submission2)
{
    const int m = submission1.size();
    const int n = submission2.size();

    // if both submissions are exactly the same, catch immediately
    if (submission1 == submission2)
    {
        return {1, m, m, 0, 0};
    }

    // TODO: Write your code here
    std::array<int, 5> result = {0, 0, 0, 0, 0};

    result[1] = totalLengthOfPatternMatches(submission1, submission2, 10);

    int start1 = 0;
    int start2 = 0;
    int longestApproximateMatch = maximumApproximatePatternMatch(submission1, submission2, start1, start2);

    result[2] = longestApproximateMatch;
    result[3] = start1;
    result[4] = start2;

    result[0] = flagged(std::min(m, n), result[1], result[2]); // flagging the files

    return result;
}
