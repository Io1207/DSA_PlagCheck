#include "plagiarism_checker.hpp"
// You should NOT add ANY other includes to this file.
// Do NOT add "using namespace std;".

// TODO: Implement the methods of the plagiarism_checker_t class

tokenized_submission_t::tokenized_submission_t(const auto &__tokens, const auto &__timestamp, const auto &__student, const auto &__professor) : tokens(__tokens), timestamp(__timestamp), student(__student), professor(__professor)
{
}

plagiarism_checker_t::plagiarism_checker_t(void) : tokenized_submissions()
{
}

// Helper function to tokenize submissions, one chunk at a time
void plagiarism_checker_t::tokenize_chunk(const std::vector<std::shared_ptr<submission_t>> &submissions, const int &start, const int &end, const auto &sub_time)
{
    for (int i = start; i < end; i++)
    {
        tokenizer_t tkzr(submissions[i]->codefile);
        auto ptr = std::make_shared<tokenized_submission_t>(tokenized_submission_t(tkzr.get_tokens(), sub_time, submissions[i]->student, submissions[i]->professor));
        // before modifying tokenized_submissions, lock the mutex so as to prevent race condition
        std::lock_guard<std::mutex> lock(sub_mutex);
        tokenized_submissions.push_back(ptr);
    }
}

// Helper function to check if two submissions are plagiarised and flag the appropriate student and professor
void plagiarism_checker_t::flag_if_plagiarised(const int &idx1, const int &idx2)
{
    // TODO: Implement this function
}

plagiarism_checker_t::plagiarism_checker_t(std::vector<std::shared_ptr<submission_t>> __submissions) : plagiarism_checker_t()
{
    // Record the submission time
    auto sub_time = std::chrono::system_clock::now();

    // Step 1: Tokenize all the new submissions and add it to tokenized_submissions

    // number of submissions
    const int n = __submissions.size();

    // Determine the number of threads to use
    const int num_threads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    int chunk_size = std::ceil((n + 0.0) / num_threads);

    // Create threads to process chunks of data
    // Each thread handles one chunk of submissions
    for (int t = 0; t < num_threads; t++)
    {
        int start = t * chunk_size;
        int end = std::min(start + chunk_size, n);
        threads.emplace_back([this, &__submissions, start, end, sub_time]()
                             { this->tokenize_chunk(__submissions, start, end, sub_time); });
    }

    // Wait for all threads to finish
    for (auto &thread : threads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }

    // Step 2: Compare each new submission with every submission before it
    for (int i = 1; i < n; i++)
    {
        for (int j = 0; j < i; j++)
        {
            flag_if_plagiarised(i, j);
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
// End TODO