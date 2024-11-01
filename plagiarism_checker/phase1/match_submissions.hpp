#include <array>
#include <iostream>
#include <span>
#include <vector>
#include <cmath>
// -----------------------------------------------------------------------------
#include <algorithm>

// You are free to add any STL includes above this comment, below the --line--.
// DO NOT add "using namespace std;" or include any other files/libraries.
// Also DO NOT add the include "bits/stdc++.h"

// OPTIONAL: Add your helper functions and data structures here

class LCSMatrix {
    int m;
    int n;
    std::vector<std::vector<int>> dp;
public:
    LCSMatrix(const std::vector<int>& vector1, const std::vector<int>& vector2) {
        m = vector1.size();
        n = vector2.size();

        // Initialize the DP table
        // dp[i][j] = LCS between vector1[:i] and vector2[:j] ending at the last index
        dp.resize(m + 1, std::vector<int>(n + 1, 0));

        // Fill DP table with lengths of common subsequences
        for (int i = 1; i <= m; i++) {
            for (int j = 1; j <= n; j++) {
                if (vector1[i - 1] == vector2[j - 1]) {
                    dp[i][j] = dp[i - 1][j - 1] + 1; // Extend the common subsequence
                } else {
                    dp[i][j] = 0; // Reset if elements don't match for continuous subsequence
                }
            }
        }
    }

    // NOTE: Currently returns an estimation of the exact match length
    // Issue lies in how to remember which indices were already counted
    int exactPatternMatchLengthSum(const int& minLength, const int& maxLength) const {
        int totalLength = 0;

        for(int i = 0; i < m; i++) {
            for(int j = 0; j < n; j++) {
                int length1 = dp[i][j];
                if(length1 >= minLength && length1 <= maxLength) {
                    std::cout << "i = " << i << ", j = " << j << ", length1 = " << length1 << std::endl;
                    // if(length1 == minLength) {
                    //     totalLength += length1;
                    // } else {
                    //     totalLength++;
                    // }
                    totalLength++;
                }
            }
        }

        return totalLength;
    }

    // NOTE: Currently returns exact match length, not approximate match length
    int longestApproximatePatternMatch(int& start1, int& start2, const double& simRatio) const {
        int maxLength = 0;

        for(int i = 1; i <= m; i++) {
            for(int j = 1; j <= n; j++) {
                int length1 = dp[i][j];
                if(length1 > maxLength) {
                    start1 = i - length1;
                    start2 = j - length1;
                    maxLength = length1;
                }
            }
        }

        return maxLength;
    }
};

// template <typename T>
// std::ostream &operator<<(std::ostream &os, const std::vector<T> &vec) {
//     os << "[";
//     for (int i = 0; i < vec.size(); i++) {
//         os << vec[i];
//         if (i != vec.size() - 1) {
//             os << ", ";
//         }
//     }
//     os << "]";
//     return os;
// }

// template <size_t N, typename T>
// std::ostream &operator<<(std::ostream &os, const std::array<T, N> &arr) {
//     os << "[";
//     for (int i = 0; i < N; i++) {
//         os << arr[i];
//         if (i != N - 1) {
//             os << ", ";
//         }
//     }
//     os << "]";
//     return os;
// }

std::array<int, 5> match_submissions(std::vector<int> &submission1, std::vector<int> &submission2) {
    // TODO: Write your code here

    LCSMatrix lcs(submission1, submission2);

    std::array<int, 5> result;

    result[1] = lcs.exactPatternMatchLengthSum(10, 20);
    
    int start1, start2;
    result[2] = lcs.longestApproximatePatternMatch(start1, start2, 0.8);
    result[3] = start1;
    result[4] = start2;

    result[0] = 0;
    
    return result;

    // End TODO
}