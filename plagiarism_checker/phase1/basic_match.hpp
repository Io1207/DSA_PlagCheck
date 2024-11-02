#include <array>
#include <iostream>
#include <span>
#include <vector>
#include <cmath>
#include <set>
#include <algorithm>

// Helper function to check if a pattern at pos1 in vec1 matches pattern at pos2 in vec2
bool isPatternMatch(const std::vector<int>& vec1, const std::vector<int>& vec2, 
                   int pos1, int pos2, int length) {
    if (pos1 + length > vec1.size() || pos2 + length > vec2.size()) 
        return false;
        
    for (int i = 0; i < length; ++i) {
        if (vec1[pos1 + i] != vec2[pos2 + i]) 
            return false;
    }
    return true;
}

// Helper function to find pattern matches of specific length
std::vector<std::pair<int, int>> findPatternsOfLength(const std::vector<int>& vec1, 
                                                     const std::vector<int>& vec2, 
                                                     int length) {
    std::vector<std::pair<int, int>> matches;
    
    // Check each possible starting position
    for (int i = 0; i <= vec1.size() - length; ++i) {
        for (int j = 0; j <= vec2.size() - length; ++j) {
            if (isPatternMatch(vec1, vec2, i, j, length)) {
                matches.push_back({i, j});
            }
        }
    }
    
    return matches;
}

int levenshteinDistance(int a, int b) {
    // Compute the absolute difference as a basic form of distance
    return std::abs(a - b);
}

int longestApproximatePatternMatch(const std::vector<int>& vector1, 
                                    const std::vector<int>& vector2, 
                                    int& startIdx1, 
                                    int& startIdx2, 
                                    double tolerance = 0.1) {
    int n = vector1.size();
    int m = vector2.size();
    
    if (n == 0 || m == 0) {
        startIdx1 = startIdx2 = -1;
        return 0;
    }
    
    int maxLen = 0;
    startIdx1 = startIdx2 = -1;

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            int matches = 0;
            int totalLen = 0;
            
            int pos1 = i;
            int pos2 = j;

            while (pos1 < n && pos2 < m) {
                // Use the Levenshtein-like distance for fuzzy matching
                if (levenshteinDistance(vector1[pos1], vector2[pos2]) <= tolerance * std::max(vector1[pos1], vector2[pos2])) {
                    matches++;
                }
                totalLen++;
                pos1++;
                pos2++;

                if (totalLen >= 30) {
                    double similarity = static_cast<double>(matches) / totalLen;
                    if (similarity >= 0.8 && totalLen > maxLen) {
                        maxLen = totalLen;
                        startIdx1 = i;
                        startIdx2 = j;
                    }
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
    
    // Calculate total length of unique pattern matches (lengths between 10 and 20)
    std::set<std::pair<int, int>> uniqueMatches;
    int totalPatternLength = 0;
    
    for (int len = 10; len <= 20; ++len) {
        auto matches = findPatternsOfLength(submission1, submission2, len);
        for (const auto& match : matches) {
            // Check if this match overlaps with any existing match
            bool isUnique = true;
            for (const auto& existing : uniqueMatches) {
                // Check for overlap in both submissions
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
    
    // Find longest approximate pattern match
    int startIdx1, startIdx2;
    result[2] = longestApproximatePatternMatch(submission1, submission2, startIdx1, startIdx2);
    
    if (result[2] >= 30) {
        result[3] = startIdx1;
        result[4] = startIdx2;
    } else {
        result[2] = 0;
        result[3] = result[4] = -1;
    }
    
    // Determine if submissions are plagiarized
    int minLength = std::min(m, n);
    int plagiarismThreshold = std::max(30, minLength / 5);  // At least 30 or 20% of smaller submission
    
    if (result[1] >= plagiarismThreshold || result[2] >= 30) {
        result[0] = 1;
    }
    
    return result;
}
