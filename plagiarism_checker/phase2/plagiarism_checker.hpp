#include "structures.hpp"
// -----------------------------------------------------------------------------
// Standard library includes
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <unordered_set>
#include <algorithm>

// You are free to add any STL includes above this comment, below the --line--.
// DO NOT add "using namespace std;" or include any other files/libraries.
// Also DO NOT add the include "bits/stdc++.h"

const int base = 257;
const int mod = 1e9 + 7;
const int num_threads = 3;

struct tokenized_submission_t : public submission_t
{
    std::vector<int> tokens;
    std::chrono::time_point<std::chrono::steady_clock> timestamp;
    std::vector<long long> hashVec;
    std::unordered_set<long long> hashSet;
    bool has_been_flagged; // To ensure we don't flag the same submission multiple times
    bool is_new_submission; // To differentiate new and pre-existing submissions
    std::mutex flag_mutex;  // Mutex for thread safety
    int patch_small;
    int patch_long;

    tokenized_submission_t(
        std::shared_ptr<submission_t> submission, const std::vector<int> &tokens,
        const std::chrono::time_point<std::chrono::steady_clock> &timestamp,
        const std::vector<long long> &hashVec, const std::unordered_set<long long> &hashSet,
        const bool &is_new_submission);
    void flag_plagiarism();
};

class plagiarism_checker_t
{
public:
    plagiarism_checker_t(void);
    plagiarism_checker_t(std::vector<std::shared_ptr<submission_t>> __submissions);
    ~plagiarism_checker_t(void);
    void add_submission(std::shared_ptr<submission_t> __submission);

protected:
    void process_submission(std::shared_ptr<submission_t> submission, std::chrono::time_point<std::chrono::steady_clock> timestamp);
    std::shared_ptr<tokenized_submission_t> get_tokenized_submission(
        const std::shared_ptr<submission_t> &submission, const std::chrono::time_point<std::chrono::steady_clock> &timestamp,
        const bool &is_new_submission);
    void tokenize_hash_chunk(const std::vector<std::shared_ptr<submission_t>> &submissions, const int &start, const int &end, const auto &timestamp);
    void plag_check_chunk(const int &start, const int &end);

    std::vector<std::shared_ptr<tokenized_submission_t>> tokenized_submissions;

    // Thread management
    std::vector<std::thread> threads;
    std::mutex sub_mutex;

    // Number of submissions
    int n;
};
