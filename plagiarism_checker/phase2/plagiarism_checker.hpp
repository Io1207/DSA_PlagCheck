#include "structures.hpp"
// -----------------------------------------------------------------------------
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <unordered_set>
#include <algorithm>
#include <queue>
#include <condition_variable>

// You are free to add any STL includes above this comment, below the --line--.
// DO NOT add "using namespace std;" or include any other files/libraries.
// Also DO NOT add the include "bits/stdc++.h"

// OPTIONAL: Add your helper functions and classes here

const int base = 257;
const int mod = 1e9 + 7;

const int num_threads = 3;

struct tokenized_submission_t : public submission_t
{
    std::vector<int> tokens;
    std::chrono::time_point<std::chrono::steady_clock> timestamp;
    std::vector<long long> hashVec;
    std::unordered_set<long long> hashSet;
    bool has_been_flagged;  // To ensure we don't flag the same submission multiple times
    bool is_new_submission; // We are not flagging pre-existing submissions that are given in the constructor
    std::mutex flag_mutex;  // To ensure thread safety
    int patch_small; //small length patch work plagiarised matches
    int patch_long; //long length patch work plagiarised matches

    tokenized_submission_t(
        std::shared_ptr<submission_t> submission, const std::vector<int> &tokens,
        const std::chrono::time_point<std::chrono::steady_clock> &timestamp,
        const std::vector<long long> &hashVec, const std::unordered_set<long long> &hashSet,
        const bool &is_new_submission);
    
    void flag_plagiarism();
};

struct waiting_submission_t : public submission_t
{
    std::chrono::time_point<std::chrono::steady_clock> timestamp;
    bool is_new_submission;

    waiting_submission_t(std::shared_ptr<submission_t> submission,
                         const std::chrono::time_point<std::chrono::steady_clock> &timestamp,
                         const bool &is_new_submission);
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

    // Member functions

    std::shared_ptr<tokenized_submission_t> get_tokenized_submission(
        const std::shared_ptr<submission_t> &submission, const std::chrono::time_point<std::chrono::steady_clock> &timestamp,
        const bool &is_new_submission);

    void process_submissions();

    // Member variables

    // Number of submissions
    int n;

    // List of tokenized submissions
    std::vector<std::shared_ptr<tokenized_submission_t>> tokenized_submissions;

    // To ensure thread safety
    std::mutex sub_mutex;

    // Queue for managing pending submissions
    std::queue<std::shared_ptr<waiting_submission_t>> process_queue;
    std::condition_variable queue_cv;

    // Worker threads
    std::thread worker_thread;

    // For signalling thread termination
    bool terminate_thread;

    // End TODO
};
