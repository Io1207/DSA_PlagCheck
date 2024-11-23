#include "plagiarism_checker.hpp"
// You should NOT add ANY other includes to this file.
// Do NOT add "using namespace std;".

// TODO: Implement the methods of the plagiarism_checker_t class

submission_wrapper::submission_wrapper(std::shared_ptr<submission_t> submission) : submission_t(*submission)
{
}

std::shared_ptr<submission_t> submission_wrapper::get_submission() const
{
    // Use shared_from_this() to create a std::shared_ptr to submission_t
    return std::shared_ptr<submission_t>(shared_from_this());
}

tokenized_submission_t::tokenized_submission_t(std::shared_ptr<submission_t> submission) : submission_wrapper(submission)
{
}

void tokenized_submission_t::flag_plagiarism()
{
    // Obtain a shared_ptr to the current object
    std::shared_ptr<submission_t> self = shared_from_this()->get_submission();

    auto student = get_submission()->student;
    auto professor = get_submission()->professor;

    if (student)
    {
        student->flag_student(self);
    }
    if (professor)
    {
        professor->flag_professor(self);
    }
}

plagiarism_checker_t::plagiarism_checker_t(void) : base(257), mod(1e9 + 7), n(0), tokenized_submissions(), sub_mutex()
{
}

plagiarism_checker_t::plagiarism_checker_t(std::vector<std::shared_ptr<submission_t>> __submissions) : plagiarism_checker_t()
{
    // Initialize the number of submissions
    n = __submissions.size();

    // Step 1: Tokenize all the new submissions and add it to tokenized_submissions

    // Record the submission time
    auto sub_time = std::chrono::system_clock::now();

    // Determine the number of threads to use
    const int num_threads = std::thread::hardware_concurrency();
    std::vector<std::thread> tokenize_threads;
    int chunk_size = std::ceil((n + 0.0) / num_threads);

    // Create threads to process chunks of data
    // Each thread handles one chunk of submissions
    for (int t = 0; t < num_threads; t++)
    {
        int start = t * chunk_size;
        int end = std::min(start + chunk_size, n);
        tokenize_threads.emplace_back([this, &__submissions, start, end, sub_time]()
                                      { this->tokenize_chunk(__submissions, start, end, sub_time); });
    }

    // Wait for all threads to finish
    for (auto &t : tokenize_threads)
    {
        if (t.joinable())
        {
            t.join();
        }
    }

    // Step 2: Hash the tokenized submissions
    // Rabin-Karp algorithm will be used to hash the tokenized submissions
    // Two window lengths, one for exact matches and one for patchwork plagiarism

    int window_size_exact = 75;
    int window_size_patchwork = 10;

    // Create threads to process chunks of data
    std::vector<std::thread> hash_threads;
    for (int t = 0; t < num_threads; t++)
    {
        hash_threads.emplace_back([this, t, chunk_size, window_size_exact, window_size_patchwork]()
                                  {
            int start = t * chunk_size;
            int end = std::min(start + chunk_size, n);
            this->hash_chunk(start, end, window_size_exact, true);
            this->hash_chunk(start, end, window_size_patchwork, false); });
    }

    // Wait for all threads to finish
    for (auto &t : hash_threads)
    {
        if (t.joinable())
        {
            t.join();
        }
    }

    // Step 3: Check for plagiarism among the submissions

    // Total number of comparisons
    int N = n * (n - 1) / 2;

    // Distribute the comparisons among threads (remainder is distributed across the first few threads)
    chunk_size = N / num_threads;
    int remainder = N % num_threads;

    // Create threads to process chunks of data
    std::vector<std::thread> plag_threads;
    int start_idx = 0;

    for (int t = 0; t < num_threads; ++t)
    {
        int end_idx = start_idx + chunk_size + (t < remainder ? 1 : 0);
        plag_threads.emplace_back([this, start_idx, end_idx]()
                                  { this->chunky_compare_pairs(start_idx, end_idx); });
        start_idx = end_idx;
    }

    // Join threads after all are finished
    for (auto &t : plag_threads)
    {
        if (t.joinable())
        {
            t.join();
        }
    }
}

plagiarism_checker_t::~plagiarism_checker_t(void)
{
}

void plagiarism_checker_t::add_submission(std::shared_ptr<submission_t> __submission)
{
    // Record the submission time
    auto sub_time = std::chrono::system_clock::now();
}

// Helper function to tokenize submissions, one chunk at a time
void plagiarism_checker_t::tokenize_chunk(const std::vector<std::shared_ptr<submission_t>> &submissions, const int &start, const int &end, const auto &sub_time)
{
    for (int i = start; i < end; i++)
    {
        tokenizer_t tkzr(submissions[i]->codefile);

        std::shared_ptr<submission_wrapper> wrapper_ptr = std::static_pointer_cast<submission_wrapper>(submissions[i]);
        std::shared_ptr<tokenized_submission_t> ptr = std::dynamic_pointer_cast<tokenized_submission_t>(wrapper_ptr);
        if (ptr)
        {
            ptr->tokens = tkzr.get_tokens();
            ptr->timestamp = sub_time;
            ptr->hashes.resize(2);
            ptr->patchwork_matches = 0;

            // before modifying tokenized_submissions, lock the mutex so as to prevent race condition
            std::lock_guard<std::mutex> lock(sub_mutex);
            tokenized_submissions.push_back(ptr);
        }
    }
}

void plagiarism_checker_t::hash_chunk(const int &start, const int &end, const int &window_size, const bool &is_exact)
{
    for (int i = start; i < end; i++)
    {
        // tokenized_submissions[i]->hashes[is_exact] = get_hashes(i, window_size);
        compute_hashes(i, window_size, is_exact);
    }
}

// Helper function to compute hashes for a submission
void plagiarism_checker_t::compute_hashes(const int &idx, const int &window_size, const bool &is_exact)
{
    long long hash = 0;
    long long base_exp = 1; // for removing the first element

    // Compute the hash of the first window
    for (int i = 0; i < window_size; i++)
    {
        hash = ((hash * base) + tokenized_submissions[idx]->tokens[i]) % mod;
        base_exp = (base_exp * base) % mod;
    }

    // Compute rolling hashes for all windows
    for (int i = window_size; i < n; i++)
    {
        tokenized_submissions[idx]->hashes[is_exact].insert(hash);
        hash = ((hash * base) + tokenized_submissions[idx]->tokens[i]) % mod;
        hash = (hash - ((tokenized_submissions[idx]->tokens[i - window_size] * base_exp) % mod) + mod) % mod;
    }

    // Add the last hash
    tokenized_submissions[idx]->hashes[is_exact].insert(hash);
}

void plagiarism_checker_t::check_exact_match(const int &i, const int &j)
{
    for (const auto &hash : tokenized_submissions[i]->hashes[1])
    {
        if (tokenized_submissions[j]->hashes[1].find(hash) != tokenized_submissions[j]->hashes[1].end())
        {

            // Check the time difference between the two submissions (in minutes)
            // If the time difference is less than 1 second, both submissions are considered to be plagiarised
            // Otherwise, only the later submission is flagged
            bool is_i_earlier = tokenized_submissions[i]->timestamp < tokenized_submissions[j]->timestamp;
            const auto &earlier_submission = is_i_earlier ? tokenized_submissions[i] : tokenized_submissions[j];
            const auto &later_submission = is_i_earlier ? tokenized_submissions[j] : tokenized_submissions[i];

            later_submission->flag_plagiarism();

            auto millisecond_diff = std::chrono::duration_cast<std::chrono::milliseconds>(later_submission->timestamp - earlier_submission->timestamp).count();
            if (millisecond_diff < 1000)
            {
                earlier_submission->flag_plagiarism();
            }
        }
    }
}

void plagiarism_checker_t::update_patchwork(const int &i, const int &j)
{
    for (const auto &hash : tokenized_submissions[i]->hashes[0])
    {
        if (tokenized_submissions[j]->hashes[0].find(hash) != tokenized_submissions[j]->hashes[0].end())
        {
            tokenized_submissions[i]->patchwork_matches++;
            tokenized_submissions[j]->patchwork_matches++;
        }
    }
}

// Helper function to compare pairs of submissions for plagiarism
void plagiarism_checker_t::chunky_compare_pairs(const int &start_idx, const int &end_idx)
{
    // Calculate the pairwise indices and call plagiarism check
    for (int idx = start_idx; idx < end_idx; idx++)
    {
        // Determine the pair (i, j) corresponding to the idx
        // idx = (n - 1) + (n - 2) + ... + (n - i) + (j - i - 1)
        // Manipulate the above equation to get i and j from idx
        int i = n - 0.5 - std::sqrt(std::pow(n - 0.5, 2) - 2 * idx);
        int j = i + 1 + (idx - 0.5 * i * (2 * n - i - 1));

        // First, we check for exact matches of length 75 or more
        check_exact_match(i, j);

        // Next, we check for patchwork plagiarism
        update_patchwork(i, j);
    }
}

// End TODO