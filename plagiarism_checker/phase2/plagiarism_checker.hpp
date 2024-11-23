#include "structures.hpp"
// -----------------------------------------------------------------------------
#include <vector>
#include <thread>
#include <mutex>
#include <cmath>
#include <chrono>
#include <unordered_set>
#include <algorithm>

// You are free to add any STL includes above this comment, below the --line--.
// DO NOT add "using namespace std;" or include any other files/libraries.
// Also DO NOT add the include "bits/stdc++.h"

// OPTIONAL: Add your helper functions and classes here

// Wrapper class to add polymorphism and enable shared_from_this
class submission_wrapper : public submission_t, public std::enable_shared_from_this<submission_wrapper> {
public:
    explicit submission_wrapper(std::shared_ptr<submission_t> submission);
    std::shared_ptr<submission_t> get_submission();
    std::shared_ptr<submission_t> get_submission() const;
};

struct tokenized_submission_t: public submission_wrapper {
    std::vector<int> tokens;
    std::chrono::time_point<std::chrono::system_clock> timestamp;
    std::vector<std::unordered_set<long long>> hashes; // 0 for patchwork, 1 for exact
    int patchwork_matches;

    tokenized_submission_t(std::shared_ptr<submission_t> submission);
    void flag_plagiarism();
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
    void hash_chunk(const int &start, const int &end, const int &window_size, const bool &is_exact);
    void compute_hashes(const int &idx, const int &window_size, const bool &is_exact);
    void chunky_compare_pairs(const int &start_idx, const int &end_idx);
    void check_exact_match(const int &i, const int &j);
    void update_patchwork(const int &i, const int &j);
    
    // Hash function parameters
    const int base;
    const int mod;

    // Number of submissions
    int n;
    
    std::vector<std::shared_ptr<tokenized_submission_t>> tokenized_submissions;
    std::mutex sub_mutex; // to ensure thread safety
    
    // End TODO
};
