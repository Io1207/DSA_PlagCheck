#include "plagiarism_checker.hpp"
// You should NOT add ANY other includes to this file.
// Do NOT add "using namespace std;".

// TODO: Implement the methods of the plagiarism_checker_t class

tokenized_submission_t::tokenized_submission_t(
    std::shared_ptr<submission_t> submission, const std::vector<int> &tokens,
    const std::chrono::time_point<std::chrono::system_clock> &timestamp,
    const std::unordered_set<long long> &exact_hashes, const std::unordered_set<long long> &patchwork_hashes)
    : submission_t(*submission), tokens(tokens), timestamp(timestamp), exact_hashes(exact_hashes),
      patchwork_hashes(patchwork_hashes), patchwork_matches(0), has_been_flagged(false)
{
}

void tokenized_submission_t::flag_plagiarism()
{
    // Don't flag the same submission multiple times
    // NOTE: Maybe we should allow multiple flags for the same submission?
    if (has_been_flagged)
    {
        return;
    }

    has_been_flagged = true;

    if (student)
    {
        student->flag_student(std::make_shared<submission_t>(*this));
    }
    if (professor)
    {
        professor->flag_professor(std::make_shared<submission_t>(*this));
    }
}

plagiarism_checker_t::plagiarism_checker_t(void)
    : num_threads(std::thread::hardware_concurrency()), base(257), mod(1e9 + 7), n(0), 
      window_size_exact(75), window_size_patchwork(10), tokenized_submissions(), submission_mutexes(), sub_mutex()
{
}

plagiarism_checker_t::plagiarism_checker_t(std::vector<std::shared_ptr<submission_t>> __submissions) : plagiarism_checker_t()
{
    // Step 0: Initializations

    auto sub_time = std::chrono::system_clock::now();

    n = __submissions.size();
    sub_mutex.lock();
    submission_mutexes = std::vector<std::mutex>(n);
    sub_mutex.unlock();

    int chunk_size = std::ceil((n + 0.0) / num_threads);

    // Step 1: Tokenize and hash all the new submissions

    std::vector<std::thread> preprocess_threads;

    // Create threads to process chunks of data
    // Each thread handles one chunk of submissions
    for (int t = 0; t < std::min(num_threads, n); t++)
    {
        int start = t * chunk_size;
        int end = std::min(start + chunk_size, n);
        preprocess_threads.emplace_back([this, &__submissions, start, end, sub_time]()
                                        { this->tokenize_hash_chunk(__submissions, start, end, sub_time); });
    }

    // Wait for all threads to finish
    for (auto &t : preprocess_threads)
    {
        t.join();
    }

    // Step 2: Check for plagiarism among the submissions

    // Total number of comparisons
    int N = n * (n - 1) / 2;

    // Distribute the comparisons among threads (remainder is distributed across the first few threads)
    chunk_size = N / num_threads;
    int remainder = N % num_threads;

    // Create threads to process chunks of data
    std::vector<std::thread> plag_threads;
    int start_idx = 0;

    for (int t = 0; t < std::min(num_threads, N); t++)
    {
        int end_idx = start_idx + chunk_size + (t < remainder ? 1 : 0);
        plag_threads.emplace_back([this, start_idx, end_idx]()
                                  { this->chunky_compare_pairs(start_idx, end_idx); });
        start_idx = end_idx;
    }

    // Join threads after all are finished
    for (auto &t : plag_threads)
    {
        t.join();
    }
}

plagiarism_checker_t::~plagiarism_checker_t(void)
{
}

void plagiarism_checker_t::add_submission(std::shared_ptr<submission_t> __submission)
{
    // Record the submission time
    auto timestamp = std::chrono::system_clock::now();

    {
        std::lock_guard<std::mutex> lock(sub_mutex);
        tokenized_submissions.push_back(get_tokenized_submission(__submission, timestamp));
        n++;
    }

    int chunk_size = std::ceil((n + 0.0) / num_threads);
    std::vector<std::thread> plag_threads;

    for (int t = 0; t < std::min(num_threads, n - 1); t++)
    {
        int start = t * chunk_size;
        int end = std::min(start + chunk_size, n - 1);
        plag_threads.emplace_back([this, start, end]()
                                  {
            for(int idx = start; idx < end; idx++) {
                check_exact_match(idx, n - 1);
                update_patchwork(idx, n - 1);
            } });
    }

    for (auto &t : plag_threads)
    {
        t.join();
    }
}

std::shared_ptr<tokenized_submission_t> plagiarism_checker_t::get_tokenized_submission(
    const std::shared_ptr<submission_t> &submission, const std::chrono::time_point<std::chrono::system_clock> &timestamp
)
{
    // Assuming required mutexes are locked before calling this function

    // Tokenize the submission
    tokenizer_t tkzr(submission->codefile);
    auto tokens = tkzr.get_tokens();
    const int num_tokens = tokens.size();

    // Hash the tokens for patchwork and exact matches
    std::unordered_set<long long> exact_hashes;
    std::unordered_set<long long> patchwork_hashes;

    long long hash_exact = 0;
    long long base_exp_exact = 1;

    long long hash_patchwork = 0;
    long long base_exp_patchwork = 1;

    for (int i = 0; i < num_tokens; i++)
    {
        if (i >= window_size_exact)
        {
            exact_hashes.insert(hash_exact);
        }
        if (i >= window_size_patchwork)
        {
            patchwork_hashes.insert(hash_patchwork);
        }

        hash_exact = ((hash_exact * base) + tokens[i]) % mod;
        hash_patchwork = ((hash_patchwork * base) + tokens[i]) % mod;

        if (i < window_size_exact)
        {
            base_exp_exact = (base_exp_exact * base) % mod;
        }
        else
        {
            hash_exact = (hash_exact - ((tokens[i - window_size_exact] * base_exp_exact) % mod) + mod) % mod;
        }
        if (i < window_size_patchwork)
        {
            base_exp_patchwork = (base_exp_patchwork * base) % mod;
        }
        else
        {
            hash_patchwork = (hash_patchwork - ((tokens[i - window_size_patchwork] * base_exp_patchwork) % mod) + mod) % mod;
        }
    }
    exact_hashes.insert(hash_exact);
    patchwork_hashes.insert(hash_patchwork);

    return std::make_shared<tokenized_submission_t>(submission, tokens, timestamp, exact_hashes, patchwork_hashes);
}

void plagiarism_checker_t::tokenize_hash_chunk(const std::vector<std::shared_ptr<submission_t>> &submissions, const int &start, const int &end, const auto &timestamp)
{
    for (int idx = start; idx < end; idx++)
    {
        std::lock_guard<std::mutex> lock(sub_mutex);
        tokenized_submissions.push_back(get_tokenized_submission(submissions[idx], timestamp));
    }
}

void plagiarism_checker_t::check_exact_match(const int &i, const int &j)
{
    for (const auto &hash : tokenized_submissions[i]->exact_hashes)
    {
        if (tokenized_submissions[j]->exact_hashes.find(hash) != tokenized_submissions[j]->exact_hashes.end())
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
    for (const auto &hash : tokenized_submissions[i]->patchwork_hashes)
    {
        if (tokenized_submissions[j]->patchwork_hashes.find(hash) != tokenized_submissions[j]->patchwork_hashes.end())
        {
            // Increment the patchwork matches for both submissions
            {
                std::lock_guard<std::mutex> lock(submission_mutexes[i]); // Scoped lock for submission i
                tokenized_submissions[i]->patchwork_matches++;
            }
            {
                std::lock_guard<std::mutex> lock(submission_mutexes[j]); // Scoped lock for submission j
                tokenized_submissions[j]->patchwork_matches++;
            }
        }
    }
}

void plagiarism_checker_t::chunky_compare_pairs(const int &start_idx, const int &end_idx)
{
    // Calculate the pairwise indices and call plagiarism check
    for (int idx = start_idx; idx < end_idx; idx++)
    {
        // Determine the pair (i, j) corresponding to the idx
        // idx = (n - 1) + (n - 2) + ... + (n - i) + (j - i - 1)
        // Manipulate the above equation to get i and j from idx
        int i = (int)(n - 0.5 - std::sqrt(std::pow(n - 0.5, 2) - 2 * idx));
        int j = (int)(i + 1 + (idx - 0.5 * i * (2 * n - i - 1)));

        // First, we check for exact matches of length 75 or more
        check_exact_match(i, j);

        // Next, we check for patchwork plagiarism
        update_patchwork(i, j);
    }
}

// End TODO