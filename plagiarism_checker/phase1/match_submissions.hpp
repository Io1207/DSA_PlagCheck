#include <array>
#include <iostream>
#include <span>
#include <vector>
#include <cmath>
// -----------------------------------------------------------------------------
#include <set>
#include <algorithm>

// You are free to add any STL includes above this comment, below the --line--.
// DO NOT add "using namespace std;" or include any other files/libraries.
// Also DO NOT add the include "bits/stdc++.h"

// OPTIONAL: Add your helper functions and data structures here

int longestApproximatePatternMatch(const std::vector<int>& vector1, const std::vector<int>& vector2, int& startIdx1, int& startIdx2) {
    int n = vector1.size();
    int m = vector2.size();

    // Edge case: if either vector is empty
    if (n == 0 || m == 0) {
        startIdx1 = startIdx2 = -1;
        return 0;
    }

    // Initialize the DP table
    std::vector<std::vector<int>> dp(n + 1, std::vector<int>(m + 1, 0));

    int maxLen = 0; // Tracks the longest approximate match length
    startIdx1 = startIdx2 = -1; // Initialize start indices of the longest match

    // Fill DP table with lengths of common subsequences
    for (int i = 1; i <= n; ++i) {
        for (int j = 1; j <= m; ++j) {
            if (vector1[i - 1] == vector2[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1] + 1; // Extend the common subsequence
            } else {
                dp[i][j] = 0; // Reset if elements don't match for continuous subsequence
            }

            // Check if this common subsequence is an approximate match
            int length1 = dp[i][j];
            if (length1 > 0) {
                int start1 = i - length1;
                int start2 = j - length1;

                // Only consider if the length meets 80% similarity criterion
                if (length1 >= 0.8 * std::max(i - start1, j - start2)) {
                    if (length1 > maxLen) {
                        maxLen = length1;
                        startIdx1 = start1;
                        startIdx2 = start2;
                    }
                }
            }
        }
    }

    return maxLen;
}

// // function to calculate nCr(x, lo) + ... + nCr(x, hi)
// long long binomialSum(const int &x, const int &lo, const int &hi) {
//     long long result = 0;
//     long long binom = 1;

//     // Initialize binom as nCr(x, lo) using iterative factorial method
//     for (int i = 0; i < lo; i++) {
//         binom *= (x - i);
//         binom /= (i + 1);
//     }

//     // Sum up binomial coefficients from nCr(x, lo) to nCr(x, hi)
//     for (int i = lo; i <= hi; i++) {
//         result += binom;
//         binom *= (x - i);
//         binom /= (i + 1);
//     }

//     return result;
// }

int sumOfUniqueMaximalPatternMatches(const std::vector<int>& vec1, const std::vector<int>& vec2) {
    int n = vec1.size();
    int m = vec2.size();

    // DP table to store lengths of common subsequences ending at each position
    std::vector<std::vector<int>> dp(n + 1, std::vector<int>(m + 1, 0));
    
    // Set to store unique maximal pattern match lengths
    std::set<int> uniqueLengths;

    for (int i = 1; i <= n; ++i) {
        for (int j = 1; j <= m; ++j) {
            if (vec1[i - 1] == vec2[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1] + 1;
                
                // Check if it's a maximal pattern match
                if ((i == n || j == m || vec1[i] != vec2[j]) && dp[i][j] > 0) {
                    uniqueLengths.insert(dp[i][j]);
                }
            } else {
                dp[i][j] = 0;
            }
        }
    }

    // Sum up all unique lengths of maximal pattern matches
    int sumOfUniqueMaximalMatches = 0;
    for (int len : uniqueLengths) {
        sumOfUniqueMaximalMatches += len;
    }

    return sumOfUniqueMaximalMatches;
}

template <typename T>
std::ostream &operator<<(std::ostream &os, const std::vector<T> &vec) {
    os << "[";
    for (int i = 0; i < vec.size(); i++) {
        os << vec[i];
        if (i != vec.size() - 1) {
            os << ", ";
        }
    }
    os << "]";
    return os;
}

template <size_t N, typename T>
std::ostream &operator<<(std::ostream &os, const std::array<T, N> &arr) {
    os << "[";
    for (int i = 0; i < N; i++) {
        os << arr[i];
        if (i != N - 1) {
            os << ", ";
        }
    }
    os << "]";
    return os;
}

std::array<int, 5> match_submissions(std::vector<int> &submission1, std::vector<int> &submission2) {
    // TODO: Write your code here
    const int m = submission1.size();
    const int n = submission2.size();
    // std::cout << "submission1 = " << submission1 << std::endl;
    // std::cout << "submission2 = " << submission2 << std::endl;

    std::array<int, 5> result = {0, 0, 0, 0, 0};
    result[1] = sumOfUniqueMaximalPatternMatches(submission1, submission2);
    int startIdx1, startIdx2;
    result[2] = longestApproximatePatternMatch(submission1, submission2, startIdx1, startIdx2);
    result[3] = startIdx1;
    result[4] = startIdx2;
    
    std::cout << "m = " << m << ", n = " << n << ", result = " << result << std::endl;
    
    // for matching subsequences whose length are between 10 and 20
    // result[1] = nCr(lcsMatrix[0][0], 10) + ... + nCr(lcsMatrix[0][0], 20);
    // std::cout << "lcsMatrix[0][0] = " << lcsMatrix[0][0] << std::endl;
    // result[1] = binomialSum(lcsMatrix[0][0], 10, 20);
    // std::cout << "result[1] = " << result[1] << std::endl;
    
    return result;

    // End TODO
}
