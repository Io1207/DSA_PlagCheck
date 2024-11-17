#include "structures.hpp"
// -----------------------------------------------------------------------------
#include <vector>
#include <thread>
#include <mutex>
#include <cmath>
#include <chrono>

// You are free to add any STL includes above this comment, below the --line--.
// DO NOT add "using namespace std;" or include any other files/libraries.
// Also DO NOT add the include "bits/stdc++.h"

// OPTIONAL: Add your helper functions and classes here

struct tokenized_submission_t {
    std::vector<int> tokens;
    std::chrono::time_point<std::chrono::system_clock> timestamp;
    std::shared_ptr<student_t> student;
    std::shared_ptr<professor_t> professor;

    tokenized_submission_t(const auto &__tokens, const auto &__timestamp, const auto &__student, const auto &__professor);
};

class plagiarism_checker_t {
    // You should NOT modify the public interface of this class.
public:
    plagiarism_checker_t(void);
    plagiarism_checker_t(std::vector<std::shared_ptr<submission_t>> 
                            __submissions);
    ~plagiarism_checker_t(void);
    void add_submission(std::shared_ptr<submission_t> __submission);

protected:
    // TODO: Add members and function signatures here
    void tokenize_chunk(const std::vector<std::shared_ptr<submission_t>> &submissions, const int &start, const int &end, const auto &sub_time);
    void flag_if_plagiarised(const int &idx1, const int &idx2);
    
    std::vector<std::shared_ptr<tokenized_submission_t>> tokenized_submissions;
    std::mutex sub_mutex;
    // End TODO
};
