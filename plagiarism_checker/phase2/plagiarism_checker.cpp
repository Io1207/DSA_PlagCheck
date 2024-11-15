#include "plagiarism_checker.hpp"
// You should NOT add ANY other includes to this file.
// Do NOT add "using namespace std;".

// TODO: Implement the methods of the plagiarism_checker_t class

plagiarism_checker_t::plagiarism_checker_t(void): tokenized_submissions() {
    
}

plagiarism_checker_t::plagiarism_checker_t(std::vector<std::shared_ptr<submission_t>> __submissions): plagiarism_checker_t() {
    for(const auto &submission: __submissions) {
        tokenizer_t tokenizer(submission->codefile);
        tokenized_submissions.push_back(tokenizer.get_tokens());
    }
}

void plagiarism_checker_t::add_submission(std::shared_ptr<submission_t> __submission) {
    tokenizers.emplace_back(__submission->codefile);
}
// End TODO