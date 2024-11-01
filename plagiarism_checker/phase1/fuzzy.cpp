#include <array>
#include <iostream>
#include <span>
#include <vector>
#include <cmath>
#include<algorithm>

int levenshteinDistance(const std::vector<int>& seq1, const std::vector<int>& seq2) 
{
    int len1 = seq1.size(), len2 = seq2.size();
    std::vector<std::vector<int>> dp(len1 + 1, std::vector<int>(len2 + 1, 0));

    for (int i = 0; i <= len1; ++i) dp[i][0] = i;
    for (int j = 0; j <= len2; ++j) dp[0][j] = j;

    for (int i = 1; i <= len1; ++i) {
        for (int j = 1; j <= len2; ++j) {
            if (seq1[i - 1] == seq2[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1];
            } else {
                dp[i][j] = std::min({dp[i - 1][j], dp[i][j - 1], dp[i - 1][j - 1]}) + 1;
            }
        }
    }

    return dp[len1][len2];
}

std::vector<std::tuple<int, int, double>> fuzzyTokenMatch(const std::vector<int>& tokens1, const std::vector<int>& tokens2, int minLength = 10, double similarityThreshold = 0.8) 
{
    std::vector<std::tuple<int, int, double>> matches;

    for (int i = 0; i <= tokens1.size() - minLength; ++i) {
        std::vector<int> subseq1(tokens1.begin() + i, tokens1.begin() + i + minLength);

        for (int j = 0; j <= tokens2.size() - minLength; ++j) {
            std::vector<int> subseq2(tokens2.begin() + j, tokens2.begin() + j + minLength);
            int editDistance = levenshteinDistance(subseq1, subseq2);
            int maxLen = std::max(subseq1.size(), subseq2.size());
            double similarity = 1.0 - (static_cast<double>(editDistance) / maxLen);

            if (similarity >= similarityThreshold) 
            {
                matches.emplace_back(i, j, similarity);
            }
        }
    }
    return matches;
}
