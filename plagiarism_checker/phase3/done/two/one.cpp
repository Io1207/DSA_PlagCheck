#include <bits/stdc++.h>
using namespace std;

class Solution {
    int ceilDivision(const int &numerator, const int &denominator) {
        int floorDiv = numerator / denominator;
        if(numerator % denominator == 0) {
            return floorDiv;
        }
        return 1 + floorDiv;
    }
    string textLeft(const vector<string> &words, const int &i, const int &j, const int &maxWidth) {
        string result = "";
        for(int k = i; k < j; k++) {
            result += words[k];
            result += ' ';
        }
        result.pop_back();
        while(result.size() < maxWidth) {
            result += ' ';
        }
        return result;
    }
    string textJustify(const vector<string> &words, const int &i, const int &j, const int &maxWidth) {
        string result = "";
        int numGaps = j - i - 1;
        int numSpaces = maxWidth;
        for(int k = i; k < j; k++) {
            numSpaces -= words[k].size();
        }
        for(int k = i; k < j; k++) {
            result += words[k];
            if(numGaps > 0) {
                int currSpaces = ceilDivision(numSpaces, numGaps);
                numSpaces -= currSpaces;
                numGaps--;
                for(int s = 0; s < currSpaces; s++) {
                    result += ' ';
                }
            }
        }
        while(numSpaces > 0) {
            result += ' ';
            numSpaces--;
        }
        return result;
    }
public:
    vector<string> fullJustify(const vector<string> &words, const int &maxWidth) {
        vector<string> result;
        int numWords = words.size();
        int i = 0;
        while(i < numWords) {
            int lineWidth = 0;
            int j = i;
            while(j < numWords && lineWidth + words[j].size() <= maxWidth) {
                lineWidth += words[j].size() + 1;
                if(lineWidth > maxWidth) {
                    lineWidth--;
                }
                j++;
            }
            if(j >= numWords) {
                result.push_back(textLeft(words, i, j, maxWidth));
            } else {
                result.push_back(textJustify(words, i, j, maxWidth));
            }
            i = j;
        }
        return result;
    }
};