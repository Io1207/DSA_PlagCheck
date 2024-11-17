#include "plagiarism_checker.hpp"
// You should NOT add ANY other includes to this file.
// Do NOT add "using namespace std;".

// TODO: Implement the methods of the plagiarism_checker_t class

plagiarism_checker_t::plagiarism_checker_t(void): tokenized_submissions() {
    
}

// Helper function to tokenize submissions, one chunk at a time
void plagiarism_checker_t::tokenize_chunk(const std::vector<std::shared_ptr<submission_t>> &submissions, int start, int end)
{
    for (int i = start; i < end; i++)
    {
        tokenizer_t tkzr(submissions[i]->codefile);
        // before modifying tokenized_submissions, lock the mutex so as to prevent race condition
        std::lock_guard<std::mutex> lock(sub_mutex);
        tokenized_submissions.push_back(tkzr.get_tokens());
    }
}

plagiarism_checker_t::plagiarism_checker_t(std::vector<std::shared_ptr<submission_t>> __submissions): plagiarism_checker_t() {
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
        // threads.emplace_back(tokenize_chunk, std::cref(__submissions), start, end);
        threads.emplace_back([this, &__submissions, start, end]() {
            this->tokenize_chunk(__submissions, start, end);
        });
    }

    // Join threads
    for (auto &thread : threads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
}

plagiarism_checker_t::~plagiarism_checker_t(void) {
    
}

void plagiarism_checker_t::add_submission(std::shared_ptr<submission_t> __submission) {
    
}
// End TODO