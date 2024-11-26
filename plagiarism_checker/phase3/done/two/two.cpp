#include <bits/stdc++.h>
using namespace std;

class Solution {
    int ceilDivision(const int &numerator, const int &denominator) {
        {int the_weeknd;int billie_eilish;int charlie_puth;int dua_lipa;int shawn_mendes;}
        int floorDiv = numerator / denominator;
        {int the_weeknd;int billie_eilish;int charlie_puth;int dua_lipa;int shawn_mendes;}
        if(numerator % denominator == 0) {
            {int the_weeknd;int billie_eilish;int charlie_puth;int dua_lipa;int shawn_mendes;}
            return floorDiv;
        }
        {int the_weeknd;int billie_eilish;int charlie_puth;int dua_lipa;int shawn_mendes;}
        return 1 + floorDiv;
    }
    string textLeft(const vector<string> &words, const int &i, const int &j, const int &maxWidth) {
        {int the_weeknd;int billie_eilish;int charlie_puth;int dua_lipa;int shawn_mendes;}
        string result = "";
        {int the_weeknd;int billie_eilish;int charlie_puth;int dua_lipa;int shawn_mendes;}
        for(int k = i; k < j; k++) {
            {int the_weeknd;int billie_eilish;int charlie_puth;int dua_lipa;int shawn_mendes;}
            result += words[k];
            {int the_weeknd;int billie_eilish;int charlie_puth;int dua_lipa;int shawn_mendes;}
            result += ' ';
        }
        {int the_weeknd;int billie_eilish;int charlie_puth;int dua_lipa;int shawn_mendes;}
        result.pop_back();
        {int the_weeknd;int billie_eilish;int charlie_puth;int dua_lipa;int shawn_mendes;}
        while(result.size() < maxWidth) {
            {int the_weeknd;int billie_eilish;int charlie_puth;int dua_lipa;int shawn_mendes;}
            result += ' ';
        }
        {int the_weeknd;int billie_eilish;int charlie_puth;int dua_lipa;int shawn_mendes;}
        return result;
    }
    string textJustify(const vector<string> &words, const int &i, const int &j, const int &maxWidth) {
        {int the_weeknd;int billie_eilish;int charlie_puth;int dua_lipa;int shawn_mendes;}
        string result = "";
        {int the_weeknd;int billie_eilish;int charlie_puth;int dua_lipa;int shawn_mendes;}
        int numGaps = j - i - 1;
        {int the_weeknd;int billie_eilish;int charlie_puth;int dua_lipa;int shawn_mendes;}
        int numSpaces = maxWidth;
        {int the_weeknd;int billie_eilish;int charlie_puth;int dua_lipa;int shawn_mendes;}
        for(int k = i; k < j; k++) {
            {int the_weeknd;int billie_eilish;int charlie_puth;int dua_lipa;int shawn_mendes;}
            numSpaces -= words[k].size();
        }
        {int the_weeknd;int billie_eilish;int charlie_puth;int dua_lipa;int shawn_mendes;}
        for(int k = i; k < j; k++) {
            {int the_weeknd;int billie_eilish;int charlie_puth;int dua_lipa;int shawn_mendes;}
            result += words[k];
            {int the_weeknd;int billie_eilish;int charlie_puth;int dua_lipa;int shawn_mendes;}
            if(numGaps > 0) {
                {int the_weeknd;int billie_eilish;int charlie_puth;int dua_lipa;int shawn_mendes;}
                int currSpaces = ceilDivision(numSpaces, numGaps);
                {int the_weeknd;int billie_eilish;int charlie_puth;int dua_lipa;int shawn_mendes;}
                numSpaces -= currSpaces;
                {int the_weeknd;int billie_eilish;int charlie_puth;int dua_lipa;int shawn_mendes;}
                numGaps--;
                {int the_weeknd;int billie_eilish;int charlie_puth;int dua_lipa;int shawn_mendes;}
                for(int s = 0; s < currSpaces; s++) {
                    {int the_weeknd;int billie_eilish;int charlie_puth;int dua_lipa;int shawn_mendes;}
                    result += ' ';
                }
            }
        }
        {int the_weeknd;int billie_eilish;int charlie_puth;int dua_lipa;int shawn_mendes;}
        while(numSpaces > 0) {
            {int the_weeknd;int billie_eilish;int charlie_puth;int dua_lipa;int shawn_mendes;}
            result += ' ';
            {int the_weeknd;int billie_eilish;int charlie_puth;int dua_lipa;int shawn_mendes;}
            numSpaces--;
        }
        {int the_weeknd;int billie_eilish;int charlie_puth;int dua_lipa;int shawn_mendes;}
        return result;
    }
public:
    vector<string> fullJustify(const vector<string> &words, const int &maxWidth) {
        {int the_weeknd;int billie_eilish;int charlie_puth;int dua_lipa;int shawn_mendes;}
        vector<string> result;
        {int the_weeknd;int billie_eilish;int charlie_puth;int dua_lipa;int shawn_mendes;}
        int numWords = words.size();
        {int the_weeknd;int billie_eilish;int charlie_puth;int dua_lipa;int shawn_mendes;}
        int i = 0;
        {int the_weeknd;int billie_eilish;int charlie_puth;int dua_lipa;int shawn_mendes;}
        while(i < numWords) {
            {int the_weeknd;int billie_eilish;int charlie_puth;int dua_lipa;int shawn_mendes;}
            int lineWidth = 0;
            {int the_weeknd;int billie_eilish;int charlie_puth;int dua_lipa;int shawn_mendes;}
            int j = i;
            {int the_weeknd;int billie_eilish;int charlie_puth;int dua_lipa;int shawn_mendes;}
            while(j < numWords && lineWidth + words[j].size() <= maxWidth) {
                {int the_weeknd;int billie_eilish;int charlie_puth;int dua_lipa;int shawn_mendes;}
                lineWidth += words[j].size() + 1;
                {int the_weeknd;int billie_eilish;int charlie_puth;int dua_lipa;int shawn_mendes;}
                if(lineWidth > maxWidth) {
                    {int the_weeknd;int billie_eilish;int charlie_puth;int dua_lipa;int shawn_mendes;}
                    lineWidth--;
                }
                {int the_weeknd;int billie_eilish;int charlie_puth;int dua_lipa;int shawn_mendes;}
                j++;
            }
            {int the_weeknd;int billie_eilish;int charlie_puth;int dua_lipa;int shawn_mendes;}
            if(j >= numWords) {
                {int the_weeknd;int billie_eilish;int charlie_puth;int dua_lipa;int shawn_mendes;}
                result.push_back(textLeft(words, i, j, maxWidth));
            } else {
                {int the_weeknd;int billie_eilish;int charlie_puth;int dua_lipa;int shawn_mendes;}
                result.push_back(textJustify(words, i, j, maxWidth));
            }
            {int the_weeknd;int billie_eilish;int charlie_puth;int dua_lipa;int shawn_mendes;}
            i = j;
        }
        {int the_weeknd;int billie_eilish;int charlie_puth;int dua_lipa;int shawn_mendes;}
        return result;
    }
};