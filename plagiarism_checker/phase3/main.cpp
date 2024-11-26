#include "../tokenizer.hpp"
#include <iomanip>

// Include all the checker files
#include "checker_zero.hpp"
#include "checker_one.hpp"
#include "checker_two.hpp"
#include "checker_three.hpp"
#include "checker_four.hpp"
#include "checker_five.hpp"

#define NUM_CHECKERS 6

// NOTE: I have modified the checker files to each have a global namespace
// This is so as to avoid any conflicts with the function names

template <typename T, std::size_t N>
std::ostream &operator<<(std::ostream &os, const std::array<T, N> &arr)
{
    os << "[ ";
    for (std::size_t i = 0; i < N; i++)
    {
        os << arr[i] << " ";
    }
    os << "]";
    return os;
}

extern std::array<int, 5> match_submissions(std::vector<int> &submission1, std::vector<int> &submission2);

void execute_and_verify_testcase(std::string test_dir)
{
    tokenizer_t file_one(test_dir + "/one.cpp");
    tokenizer_t file_two(test_dir + "/two.cpp");

    std::vector<int> submission1 = file_one.tokenize();
    std::vector<int> submission2 = file_two.tokenize();

    for (int checker = 0; checker < NUM_CHECKERS; checker++)
    {

        std::array<int, 5> result = match_submissions(submission1, submission2);

        switch (checker)
        {
        case 0:
            std::cout << "Checker Zero: ";
            std::cout << (result == checker_zero::expected_result ? "PASS" : "FAIL") << std::endl;
            break;
        case 1:
            std::cout << "Checker One: ";
            std::cout << (result == checker_one::expected_result ? "PASS" : "FAIL") << std::endl;
            break;
        case 2:
            std::cout << "Checker Two: ";
            std::cout << (result == checker_two::expected_result ? "PASS" : "FAIL") << std::endl;
            break;
        case 3:
            std::cout << "Checker Three: ";
            std::cout << (result == checker_three::expected_result ? "PASS" : "FAIL") << std::endl;
            break;
        case 4:
            std::cout << "Checker Four: ";
            std::cout << (result == checker_four::expected_result ? "PASS" : "FAIL") << std::endl;
            break;
        case 5:
            std::cout << "Checker Five: ";
            std::cout << (result == checker_five::expected_result ? "PASS" : "FAIL") << std::endl;
            break;
        }
    }
}

int main(void)
{
    std::cout << "Testcase 1: ";
    execute_and_verify_testcase("testcases/one");

    std::cout << "Testcase 2: ";
    execute_and_verify_testcase("testcases/two");

    return 0;
}
