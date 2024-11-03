#include <array>
#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include <cmath>

bool isPatternMatch(const std::vector<int>& vec1, const std::vector<int>& vec2, int pos1, int pos2, int length) {
    if (pos1 + length > vec1.size() || pos2 + length > vec2.size()) 
        return false;

    for (int i = 0; i < length; ++i) {
        if (vec1[pos1 + i] != vec2[pos2 + i]) 
            return false;
    }
    return true;
}

std::vector<std::pair<int, int>> findPatternsOfLength(const std::vector<int>& vec1, const std::vector<int>& vec2, int length) {
    std::vector<std::pair<int, int>> matches;
    for (int i = 0; i <= vec1.size() - length; ++i) {
        for (int j = 0; j <= vec2.size() - length; ++j) {
            if (isPatternMatch(vec1, vec2, i, j, length)) {
                matches.push_back({i, j});
            }
        }
    }
    return matches;
}

double normalizedLevenshteinDistance(int a, int b) {
    return static_cast<double>(std::abs(a - b)) / std::max(a, b);
}

int longestApproximatePatternMatch(const std::vector<int>& vector1, const std::vector<int>& vector2, int& startIdx1, int& startIdx2, double tolerance = 0.1) {
    int maxLen = 0;
    startIdx1 = startIdx2 = -1;

    for (int i = 0; i < vector1.size(); ++i) {
        for (int j = 0; j < vector2.size(); ++j) {
            int matches = 0;
            int len = 0;
            int currentStartIdx1 = i;
            int currentStartIdx2 = j;

            while (currentStartIdx1 + len < vector1.size() && currentStartIdx2 + len < vector2.size()) {
                int val1 = vector1[currentStartIdx1 + len];
                int val2 = vector2[currentStartIdx2 + len];

                if (normalizedLevenshteinDistance(val1, val2) <= tolerance) {
                    matches++;
                }
                
                len++;
                double similarity = static_cast<double>(matches) / len;

                if (similarity >= 0.8) {
                    if (len > maxLen) {
                        maxLen = len;
                        startIdx1 = currentStartIdx1;
                        startIdx2 = currentStartIdx2;
                    }
                } else {
                    // Reset if similarity threshold is not met
                    matches = 0;
                    len = 0;
                    break;
                }
            }
        }
    }
    return maxLen;
}



std::array<int, 5> match_submissions(std::vector<int>& submission1, std::vector<int>& submission2) {
    const int m = submission1.size();
    const int n = submission2.size();
    std::array<int, 5> result = {0, 0, 0, 0, 0};
    
    std::set<std::pair<int, int>> uniqueMatches;
    int totalPatternLength = 0;

    for (int len = 10; len <= 20; ++len) {
        auto matches = findPatternsOfLength(submission1, submission2, len);
        for (const auto& match : matches) {
            bool isUnique = true;
            for (const auto& existing : uniqueMatches) {
                bool overlap1 = (match.first >= existing.first && match.first < existing.first + len);
                bool overlap2 = (match.second >= existing.second && match.second < existing.second + len);

                if (overlap1 || overlap2) {
                    isUnique = false;
                    break;
                }
            }
            
            if (isUnique) {
                uniqueMatches.insert(match);
                totalPatternLength += len;
            }
        }
    }
    
    result[1] = totalPatternLength;

    int startIdx1, startIdx2;
    result[2] = longestApproximatePatternMatch(submission1, submission2, startIdx1, startIdx2);
    
    if (result[2] >= 30) {
        result[3] = startIdx1;
        result[4] = startIdx2;
    } else {
        result[2] = 0;
        result[3] = result[4] = -1;
    }
    
    int minLength = std::min(m, n);
    int plagiarismThreshold = std::max(30, minLength / 5);
    
    if (result[1] >= plagiarismThreshold || result[2] >= 30) {
        result[0] = 1;
    }
    
    return result;
}