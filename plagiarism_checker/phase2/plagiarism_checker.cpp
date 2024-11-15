#include "plagiarism_checker.hpp"
// You should NOT add ANY other includes to this file.
// Do NOT add "using namespace std;".

// TODO: Implement the methods of the plagiarism_checker_t class

plagiarism_checker_t::plagiarism_checker_t(void): tokenized_submissions(), threads() {
    
}

plagiarism_checker_t::plagiarism_checker_t(std::vector<std::shared_ptr<submission_t>> __submissions): plagiarism_checker_t() {
    for(const auto &submission: __submissions) {
        threads.emplace_back(&plagiarism_checker_t::add_submission, this, submission);
        // add_submission(submission);
    }
}

plagiarism_checker_t::~plagiarism_checker_t(void) {
    // Wait for all threads to complete
    for (auto &thread : threads) {
        if(thread.joinable()) {
            thread.join(); 
        }
    }
}

void plagiarism_checker_t::add_submission(std::shared_ptr<submission_t> __submission) {
    tokenizer_t tokenizer(__submission->codefile);
    auto tokens = tokenizer.get_tokens(); // Perform the computation outside the mutex
    {
        std::lock_guard<std::mutex> lock(sub_mutex);
        tokenized_submissions.push_back(tokens);
    }
}
// End TODO