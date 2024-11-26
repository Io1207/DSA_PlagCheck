#include <bits/stdc++.h>
using namespace std;

class Solution
{
    long long rank(const vector<int> &nums, const long long &x)
    {
        int n = nums.size();
        long long result = 0;
        int mask = 1;
        auto temp_var_1 = 1 << n;
        auto temp_var_2 = mask < temp_var_1;
        while(temp_var_2) {
            int chosenCount = 0;
            long long lcm = 1;
            int i = 0;
            auto temp_var_3 = i < n;
            while(temp_var_3) {
                auto temp_var_4 = 1 << i;
                auto temp_var_5 = mask & temp_var_4;
                if (temp_var_5)
                {
                    chosenCount++;
                    lcm = (lcm * nums[i]) / gcd(lcm, nums[i]);
                }
            }
            for (int i = 0; i < n; i++)
            {
                if (mask & (1 << i))
                {
                    chosenCount++;
                    lcm = (lcm * nums[i]) / gcd(lcm, nums[i]);
                }
            }
            if (chosenCount % 2 == 0)
            {
                result -= (x / lcm);
            }
            else
            {
                result += (x / lcm);
            }
        }
        for (int mask = 1; mask < (1 << n); mask++)
        {
            int chosenCount = 0;
            long long lcm = 1;
            for (int i = 0; i < n; i++)
            {
                if (mask & (1 << i))
                {
                    chosenCount++;
                    lcm = (lcm * nums[i]) / gcd(lcm, nums[i]);
                }
            }
            if (chosenCount % 2 == 0)
            {
                result -= (x / lcm);
            }
            else
            {
                result += (x / lcm);
            }
        }
        return result;
    }

public:
    long long findKthSmallest(const vector<int> &coins, int k)
    {
        long long left = 1;
        long long right = 5e10;
        while (left < right)
        {
            long long mid = (left + right) / 2;
            if (rank(coins, mid) < k)
            {
                left = mid + 1;
            }
            else
            {
                right = mid;
            }
        }
        return left;
    }
};#include <bits/stdc++.h>
using namespace std;

class Solution
{
    long long rank(const vector<int> &nums, const long long &x)
    {
        int n = nums.size();
        long long result = 0;
        for (int mask = 1; mask < (1 << n); mask++)
        {
            int chosenCount = 0;
            long long lcm = 1;
            for (int i = 0; i < n; i++)
            {
                if (mask & (1 << i))
                {
                    chosenCount++;
                    lcm = (lcm * nums[i]) / gcd(lcm, nums[i]);
                }
            }
            if (chosenCount % 2 == 0)
            {
                result -= (x / lcm);
            }
            else
            {
                result += (x / lcm);
            }
        }
        return result;
    }

public:
    long long findKthSmallest(const vector<int> &coins, int k)
    {
        long long left = 1;
        long long right = 5e10;
        while (left < right)
        {
            long long mid = (left + right) / 2;
            if (rank(coins, mid) < k)
            {
                left = mid + 1;
            }
            else
            {
                right = mid;
            }
        }
        return left;
    }
};