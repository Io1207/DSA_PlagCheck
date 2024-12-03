#include <bits/stdc++.h>
using namespace std;

class Solution
{
    vector<vector<int>> memo;
    int min(const int &a, const int &b, const int &c)
    {
        if (a < b)
        {
            return (a < c) ? a : c;
        }
        return (b < c) ? b : c;
    }
    int dfs(const string &word1, const string &word2, int i, int j)
    {
        while (i < word1.size() && j < word2.size() && word1[i] == word2[j])
        {
            i++;
            j++;
        }
        if (i >= word1.size())
        {
            return word2.size() - j;
        }
        if (j >= word2.size())
        {
            return word1.size() - i;
        }
        if (memo[i][j] >= 0)
        {
            return memo[i][j];
        }
        memo[i][j] = 1 + min(dfs(word1, word2, i, j + 1), dfs(word1, word2, i + 1, j), dfs(word1, word2, i + 1, j + 1));
        return memo[i][j];
    }

public:
    int minDistance(const string &word1, const string &word2)
    {
        memo.resize(word1.size(), vector<int>(word2.size(), -1));
        return dfs(word1, word2, 0, 0);

        // O(n) time
        // O(m*n) space
    }
};