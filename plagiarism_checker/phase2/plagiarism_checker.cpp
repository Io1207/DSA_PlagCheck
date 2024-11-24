#include "plagiarism_checker.hpp"

// -----------------------------------------------------------------------------
// Implementation of the tokenized_submission_t class

tokenized_submission_t::tokenized_submission_t(
    std::shared_ptr<submission_t> submission, const std::vector<int> &tokens,
    const std::chrono::time_point<std::chrono::steady_clock> &timestamp,
    const std::vector<long long> &hashVec, const std::unordered_set<long long> &hashSet,
    const bool &is_new_submission)
    : submission_t(*submission), tokens(tokens), timestamp(timestamp), hashVec(hashVec),
      hashSet(hashSet), has_been_flagged(false), is_new_submission(is_new_submission),
      patch_small(0), patch_long(0)
{
}

void tokenized_submission_t::flag_plagiarism()
{
    std::lock_guard<std::mutex> lock(flag_mutex);
    if (has_been_flagged || !is_new_submission)
        return;

    has_been_flagged = true;
    if (student)
        student->flag_student(std::make_shared<submission_t>(*this));
    if (professor)
        professor->flag_professor(std::make_shared<submission_t>(*this));
}

// -----------------------------------------------------------------------------
// Implementation of the plagiarism_checker_t class

plagiarism_checker_t::plagiarism_checker_t(void)
    : n(0), tokenized_submissions(), sub_mutex()
{
}

plagiarism_checker_t::plagiarism_checker_t(std::vector<std::shared_ptr<submission_t>> __submissions)
    : plagiarism_checker_t()
{
    auto timestamp = std::chrono::steady_clock::now();
    n = __submissions.size();

    // Step 1: Tokenize all submissions
    for (auto &submission : __submissions)
    {
        tokenized_submissions.push_back(get_tokenized_submission(submission, timestamp, false));
    }

    // Step 2: Perform plagiarism checks in parallel
    int chunk_size = n / num_threads;
    int remainder = n % num_threads;
    int start = 0;

    for (int t = 0; t < num_threads; ++t)
    {
        int end = start + chunk_size + (t < remainder);
        threads.emplace_back(&plagiarism_checker_t::plag_check_chunk, this, start, end);
        start = end;
    }

    for (auto &t : threads)
        t.join();
}

plagiarism_checker_t::~plagiarism_checker_t(void)
{
    for (auto &t : threads)
    {
        if (t.joinable())
            t.join();
    }
}

void plagiarism_checker_t::add_submission(std::shared_ptr<submission_t> __submission)
{
    auto timestamp = std::chrono::steady_clock::now();

    // Step 1: Tokenize the new submission
    auto ptr = get_tokenized_submission(__submission, timestamp, true);

    {
        std::lock_guard<std::mutex> lock(sub_mutex);
        tokenized_submissions.push_back(ptr);
        n++;
    }

    // Step 2: Perform plagiarism check
    int chunk_size = n / num_threads;
    int remainder = n % num_threads;
    int start = 0;

    for (int t = 0; t < num_threads; ++t)
    {
        int end = start + chunk_size + (t < remainder);
        threads.emplace_back(&plagiarism_checker_t::plag_check_chunk, this, start, end);
        start = end;
    }

    for (auto &t : threads)
    {
        if (t.joinable())
            t.join();
    }
}

std::shared_ptr<tokenized_submission_t> plagiarism_checker_t::get_tokenized_submission(
    const std::shared_ptr<submission_t> &submission, const std::chrono::time_point<std::chrono::steady_clock> &timestamp,
    const bool &is_new_submission)
{
    tokenizer_t tkzr(submission->codefile);
    auto tokens = tkzr.get_tokens();
    const int length = tokens.size();

    const int minLength = 15;
    std::vector<long long> hashVec(length - minLength + 1);
    std::unordered_set<long long> hashSet;

    long long hash = 0;
    long long baseExp = 1;

    for (int i = 0; i < minLength; ++i)
    {
        hash = ((hash * base) + tokens[i]) % mod;
        baseExp = (baseExp * base) % mod;
    }

    for (int i = minLength; i < length; ++i)
    {
        hashVec[i - minLength] = hash;
        hashSet.insert(hash);
        hash = ((hash * base) + tokens[i]) % mod;
        hash = (hash - ((tokens[i - minLength] * baseExp) % mod) + mod) % mod;
    }

    hashVec[length - minLength] = hash;
    hashSet.insert(hash);

    return std::make_shared<tokenized_submission_t>(submission, tokens, timestamp, hashVec, hashSet, is_new_submission);
}

void plagiarism_checker_t::tokenize_hash_chunk(const std::vector<std::shared_ptr<submission_t>> &submissions, const int &start, const int &end, const auto &timestamp)
{
    for (int idx = start; idx < end; ++idx)
    {
        std::lock_guard<std::mutex> lock(sub_mutex);
        tokenized_submissions.push_back(get_tokenized_submission(submissions[idx], timestamp, false));
    }
}

void plagiarism_checker_t::plag_check_chunk(const int &start, const int &end)
{
    for (int i = start; i < end; ++i)
    {
        for (int j = i + 1; j < n; ++j)
        {
            totalLengthOfPatternMatches(15, tokenized_submissions[i], tokenized_submissions[j]);
        }
    }
}
