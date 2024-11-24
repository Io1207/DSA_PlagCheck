#include "plagiarism_checker.hpp"
// You should NOT add ANY other includes to this file.
// Do NOT add "using namespace std;".

// TODO: Implement the methods of the plagiarism_checker_t class

tokenized_submission_t::tokenized_submission_t(
    std::shared_ptr<submission_t> submission, const std::vector<int> &tokens,
    const std::chrono::time_point<std::chrono::system_clock> &timestamp,
    const std::unordered_set<long long> &exact_hashes, const std::unordered_set<long long> &patchwork_hashes,
    const bool &is_new_submission)
    : submission_t(*submission), tokens(tokens), timestamp(timestamp), exact_hashes(exact_hashes),
      patchwork_hashes(patchwork_hashes), patchwork_matches(0), has_been_flagged(false), is_new_submission(is_new_submission)
{
}

void tokenized_submission_t::flag_plagiarism()
{
    std::lock_guard<std::mutex> lock(flag_mutex);

    // Don't flag the same submission multiple times
    // Similarly, don't flag pre-existing submissions
    if (has_been_flagged || !is_new_submission)
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

// We are allowed the use of upto 3 threads only
plagiarism_checker_t::plagiarism_checker_t(void)
    : num_threads(3), base(257), mod(1e9 + 7), n(0),
      window_size_exact(75), window_size_patchwork(10), tokenized_submissions(), sub_mutex()
{
}

plagiarism_checker_t::plagiarism_checker_t(std::vector<std::shared_ptr<submission_t>> __submissions) : plagiarism_checker_t()
{
    // Step 1: Initializations

    auto timestamp = std::chrono::system_clock::now();
    n = __submissions.size();

    int chunk_size = n / num_threads;
    int remainder = n % num_threads;

    // Step 2: Tokenize and hash all the new submissions

    std::vector<std::thread> preprocess_threads;

    // Create threads to process chunks of data
    // Each thread handles one chunk of submissions
    int start = 0;
    for (int t = 0; t < std::min(num_threads, n); t++)
    {
        int end = start + chunk_size + (t < remainder);
        preprocess_threads.emplace_back([this, &__submissions, start, end, timestamp]()
                                        { this->tokenize_hash_chunk(__submissions, start, end, timestamp); });
        start = end;
    }

    // Wait for all threads to finish
    for (auto &t : preprocess_threads)
    {
        t.join();
    }
}

plagiarism_checker_t::~plagiarism_checker_t(void)
{
}

void plagiarism_checker_t::add_submission(std::shared_ptr<submission_t> __submission)
{
    std::cerr << "Adding new submission" << std::endl;
    // Record the submission time
    auto timestamp = std::chrono::system_clock::now();

    // Preprocessing of the new submissions can occur in parallel
    // NOTE: Potentially in the future, if required

    // Preprocess the new submission
    auto ptr = get_tokenized_submission(__submission, timestamp, true);

    // Add the new submission to the list of tokenized submissions
    {
        std::lock_guard<std::mutex> lock(sub_mutex);
        tokenized_submissions.push_back(ptr);
        n++;
    }

    // Check for plagiarism with all the existing submissions
    int chunk_size = (n - 1) / num_threads;
    int remainder = (n - 1) % num_threads;
    
    std::vector<std::thread> plag_threads;
    int start = 0;

    for (int t = 0; t < std::min(num_threads, n - 1); t++)
    {
        int end = start + chunk_size + (t < remainder);
        
        plag_threads.emplace_back([this, start, end]()
                                  {
            std::cerr << "[" << std::this_thread::get_id() << "] Checking " << n - 1 << " against chunk (" << start << ", " << end << ")" << std::endl;
            for(int idx = start; idx < end; idx++) {
                check_exact_match(idx, n - 1);
                // NOTE: Do the patchwork plagiarism check here
            } });
        
        start = end;
    }

    for (auto &t : plag_threads)
    {
        t.join();
    }
}

std::shared_ptr<tokenized_submission_t> plagiarism_checker_t::get_tokenized_submission(
    const std::shared_ptr<submission_t> &submission, const std::chrono::time_point<std::chrono::system_clock> &timestamp,
    const bool &is_new_submission)
{
    std::cerr << "[" << std::this_thread::get_id() << "] Preprocessing submission" << std::endl;

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

    return std::make_shared<tokenized_submission_t>(submission, tokens, timestamp, exact_hashes, patchwork_hashes, is_new_submission);
}

void plagiarism_checker_t::tokenize_hash_chunk(const std::vector<std::shared_ptr<submission_t>> &submissions, const int &start, const int &end, const auto &timestamp)
{
    std::cerr << "[" << std::this_thread::get_id() << "] Preprocessing chunk (" << start << ", " << end << ")" << std::endl;
    for (int idx = start; idx < end; idx++)
    {
        std::lock_guard<std::mutex> lock(sub_mutex);
        tokenized_submissions.push_back(get_tokenized_submission(submissions[idx], timestamp, false));
    }
}

void plagiarism_checker_t::check_exact_match(const int &i, const int &j)
{
    std::cerr << "[" << std::this_thread::get_id() << "] Checking exact match between " << i << " and " << j << std::endl;

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

// End TODO