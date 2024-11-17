#include "structures.hpp"
// -----------------------------------------------------------------------------
#include <vector>
#include <thread>
#include <mutex>
#include <cmath>

// You are free to add any STL includes above this comment, below the --line--.
// DO NOT add "using namespace std;" or include any other files/libraries.
// Also DO NOT add the include "bits/stdc++.h"

// OPTIONAL: Add your helper functions and classes here

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
    void tokenize_chunk(const std::vector<std::shared_ptr<submission_t>> &submissions, int start, int end);
    
    std::vector<std::vector<int>> tokenized_submissions;
    // std::vector<std::thread> threads;
    std::mutex sub_mutex;
    // End TODO
};
