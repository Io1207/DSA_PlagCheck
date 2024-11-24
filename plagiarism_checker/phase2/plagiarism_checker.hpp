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

struct tokenized_submission_t : public submission_t
{
    std::vector<int> tokens;
    std::chrono::time_point<std::chrono::system_clock> timestamp;
    std::unordered_set<long long> exact_hashes;
    std::unordered_set<long long> patchwork_hashes;
    int patchwork_matches;
    bool has_been_flagged; // To ensure we don't flag the same submission multiple times
    bool is_new_submission; // We are not flagging pre-existing submissions that are given in the constructor

    tokenized_submission_t(
        std::shared_ptr<submission_t> submission, const std::vector<int> &tokens, 
        const std::chrono::time_point<std::chrono::system_clock> &timestamp, 
        const std::unordered_set<long long> &exact_hashes, const std::unordered_set<long long> &patchwork_hashes,
        const bool &is_new_submission = false
    );
    void flag_plagiarism();
};

class plagiarism_checker_t
{
    // You should NOT modify the public interface of this class.
public:
    plagiarism_checker_t(void);
    plagiarism_checker_t(std::vector<std::shared_ptr<submission_t>>
                             __submissions);
    ~plagiarism_checker_t(void);
    void add_submission(std::shared_ptr<submission_t> __submission);

protected:
    // TODO: Add members and function signatures here

    std::shared_ptr<tokenized_submission_t> get_tokenized_submission(
        const std::shared_ptr<submission_t> &submission, const std::chrono::time_point<std::chrono::system_clock> &timestamp
    );
    void tokenize_hash_chunk(const std::vector<std::shared_ptr<submission_t>> &submissions, const int &start, const int &end, const auto &timestamp);
    void chunky_compare_pairs(const int &start_idx, const int &end_idx);

    void check_exact_match(const int &i, const int &j);
    void update_patchwork(const int &i, const int &j);

    // Number of threads for parallel processing
    int num_threads;

    // Hash function parameters
    const int base;
    const int mod;

    // Number of submissions
    int n;

    // Detection threshold parameters
    const int window_size_exact;
    const int window_size_patchwork;

    std::vector<std::shared_ptr<tokenized_submission_t>> tokenized_submissions;

    // To ensure thread safety
    std::mutex sub_mutex;
    // std::mutex patchwork_mutex; 

    // End TODO
};
