#include "plagiarism_checker.hpp"
// You should NOT add ANY other includes to this file.
// Do NOT add "using namespace std;".

// TODO: Implement the methods of the plagiarism_checker_t class

void totalLengthOfPatternMatches(const int &minLength, std::shared_ptr<tokenized_submission_t> sub1, std::shared_ptr<tokenized_submission_t> sub2)
{
    {
        std::lock_guard<std::mutex> lock1(sub1->flag_mutex);
        std::lock_guard<std::mutex> lock2(sub2->flag_mutex);
        if (sub1->has_been_flagged && sub2->has_been_flagged)
            return;
    }
    if (!sub1->is_new_submission && !sub2->is_new_submission)
        return;

    const std::vector<int> &submission1 = sub1->tokens;
    const std::vector<int> &submission2 = sub2->tokens;
    const std::vector<long long> &hashAt1 = sub1->hashVec;
    const std::unordered_set<long long> &hashes2 = sub2->hashSet;
    const int m = submission1.size();

    if (submission1 == submission2)
    {
        auto timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(sub1->timestamp - sub2->timestamp).count();
        if (timeDiff > 1000)
        {
            // std::cerr << "submission1==submission2 and timeDiff > 1000" << std::endl;
            sub1->flag_plagiarism();
        }
        else if (timeDiff < -1000)
        {
            // std::cerr << "submission1==submission2 and timeDiff < -1000" << std::endl;
            sub2->flag_plagiarism();
        }
        else
        {
            // std::cerr << "submission1==submission2 and timeDiff in range" << std::endl;
            sub1->flag_plagiarism();
            sub2->flag_plagiarism();
        }
        return;
    }

    // make counted1, a vector of booleans of size m, all set to false initially
    // counted1[i] is set to true if submmission1[i] is part of a match
    std::vector<bool> counted1(m, false);

    // for each hash in hashAt1, check if it is in hashes2
    // if it is, set all the corresponding values in counted1 to true
    for (int i = 0; i < m - minLength + 1; i++)
    {
        if (counted1[i])
            continue;
        if (hashes2.find(hashAt1[i]) != hashes2.end())
        {
            for (int i1 = i; i1 < i + minLength; i1++)
            {
                counted1[i1] = true;
            }
        }
    }

    int countSmall = 0;
    int countLong = 0;
    int count = 0;
    for (int i = 0; i < m; i++)
    {
        if (counted1[i])
        {
            count++;
        }
        else
        {
            if (count >= 75)
            {
                auto timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(sub1->timestamp - sub2->timestamp).count();
                std::cerr << "timeDiff = " << timeDiff << std::endl;
                if (timeDiff > 1000)
                {
                    // std::cerr << "count==75 and timeDiff > 1000" << std::endl;
                    sub1->flag_plagiarism();
                }
                else if (timeDiff < -1000)
                {
                    // std::cerr << "count==75 and timeDiff < -1000" << std::endl;
                    sub2->flag_plagiarism();
                }
                else
                {
                    // std::cerr << "count==75 and timeDiff in range" << std::endl;
                    sub1->flag_plagiarism();
                    sub2->flag_plagiarism();
                }
                return;
            }
            else if (count >= 25)
            {
                countLong++;
            }
            else if (count >= minLength)
            {
                countSmall++;
            }
            count = 0;
        }
    }

    auto timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(sub1->timestamp - sub2->timestamp).count();
    // if (countSmall >= 10 || countLong >= 7)
    if (countSmall + countLong >= 10 || countLong >= 6 || 10 * countSmall + 25 * countLong >= 150)
    {
        if (timeDiff > 1000)
        {
            // std::cerr << "countSmall >= 10, timeDiff > 1000" << std::endl;
            sub1->flag_plagiarism();
        }
        else if (timeDiff < -1000)
        {
            // std::cerr << "countSmall >= 10, timeDiff < -1000" << std::endl;
            sub2->flag_plagiarism();
        }
        else
        {
            // std::cerr << "countSmall >= 10, timeDiff in range" << std::endl;
            sub1->flag_plagiarism();
            sub2->flag_plagiarism();
        }
    }

    // Checking for patchwork plagiarism
    for (const auto &sub : {sub1, sub2})
    {
        if (sub->is_new_submission)
        {
            std::lock_guard<std::mutex> lock(sub->flag_mutex);
            sub->patch_small = sub->patch_small + countSmall;
            sub->patch_long = sub->patch_long + countLong;
            int length = 300;
            int checking = minLength * sub->patch_small + 25 * sub->patch_long;
            // 3 checks: number of small matches, number of long matches, total length of matches
            // if (sub->patch_small >= 20 || sub->patch_long >= 15 || checking > length)
            if (sub->patch_small + sub->patch_long >= 20 || checking >= length || sub->patch_long >= 12)
            {
                // std::cerr << "patch_small >= 20" << std::endl;
                sub->flag_plagiarism();
            }
        }
    }

    return;
}

////////////////////////// IMPLEMENTATION //////////////////////////

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
    // Lock the mutex to ensure thread safety for checking and modifying has_been_flagged
    std::lock_guard<std::mutex> lock(flag_mutex);

    // Don't flag the same submission multiple times or pre-existing submissions
    if (has_been_flagged || !is_new_submission)
    {
        return;
    }

    // Set has_been_flagged to true while the mutex is locked
    has_been_flagged = true;

    // Flag the student and professor if needed
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
    : threads(), n(0), tokenized_submissions(), sub_mutex()
{
}

plagiarism_checker_t::plagiarism_checker_t(std::vector<std::shared_ptr<submission_t>> __submissions) : plagiarism_checker_t()
{
    // Step 1: Initializations

    auto timestamp = std::chrono::steady_clock::now();
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
    for (auto &t : threads)
    {
        t.join();
    }
}

void plagiarism_checker_t::process_submission(std::shared_ptr<submission_t> __submission, std::chrono::time_point<std::chrono::steady_clock> timestamp) {
    auto ptr = get_tokenized_submission(__submission, timestamp, true);

    // Add the new submission to the list of tokenized submissions
    {
        std::lock_guard<std::mutex> lock(sub_mutex);
        tokenized_submissions.push_back(ptr);
        n++;
    }

    // Check for plagiarism with all the existing submissions
    int chunk_size = (n - 1) / (num_threads - 1);
    int remainder = (n - 1) % (num_threads - 1);

    int start = 0;

    for (int t = 0; t < std::min(num_threads - 1, n - 1); t++)
    {
        int end = start + chunk_size + (t < remainder);

        threads.emplace_back([this, start, end]()
                                  {
            // std::cerr << "[" << std::this_thread::get_id() << "] Checking " << n - 1 << " against chunk (" << start << ", " << end << ")" << std::endl;
            for(int idx = start; idx < end; idx++) {
                totalLengthOfPatternMatches(15, tokenized_submissions[idx], tokenized_submissions[n - 1]);
            } });

        start = end;
    }
}

void plagiarism_checker_t::add_submission(std::shared_ptr<submission_t> __submission)
{
    // Record the submission time
    auto timestamp = std::chrono::steady_clock::now();
    threads.push_back(std::thread(&plagiarism_checker_t::process_submission, this, __submission, timestamp));
}

std::shared_ptr<tokenized_submission_t> plagiarism_checker_t::get_tokenized_submission(
    const std::shared_ptr<submission_t> &submission, const std::chrono::time_point<std::chrono::steady_clock> &timestamp,
    const bool &is_new_submission)
{
    // std::cerr << "[" << std::this_thread::get_id() << "] Preprocessing submission" << std::endl;

    // Assuming required mutexes are locked before calling this function

    // Tokenize the submission
    tokenizer_t tkzr(submission->codefile);
    auto tokens = tkzr.get_tokens();
    const int length = tokens.size();

    // Get the hashes for each index as well as the set of all hashes

    const int minLength = 15;
    std::vector<long long> hashVec(length - minLength + 1);
    std::unordered_set<long long> hashSet;

    long long hash = 0;
    long long baseExp = 1; // to remove the first element

    // Compute the hash of the first window
    for (int i = 0; i < minLength; i++)
    {
        hash = ((hash * base) + tokens[i]) % mod;
        baseExp = (baseExp * base) % mod;
    }

    // Compute rolling hashes for all windows
    for (int i = minLength; i < length; ++i)
    {
        hashVec[i - minLength] = hash;
        hashSet.insert(hash);
        hash = ((hash * base) + tokens[i]) % mod;
        hash = (hash - ((tokens[i - minLength] * baseExp) % mod) + mod) % mod;
    }

    // Add the last hash
    hashVec[length - minLength] = hash;
    hashSet.insert(hash);

    return std::make_shared<tokenized_submission_t>(submission, tokens, timestamp, hashVec, hashSet, is_new_submission);
}

void plagiarism_checker_t::tokenize_hash_chunk(const std::vector<std::shared_ptr<submission_t>> &submissions, const int &start, const int &end, const auto &timestamp)
{
    // std::cerr << "[" << std::this_thread::get_id() << "] Preprocessing chunk (" << start << ", " << end << ")" << std::endl;
    for (int idx = start; idx < end; idx++)
    {
        std::lock_guard<std::mutex> lock(sub_mutex);
        tokenized_submissions.push_back(get_tokenized_submission(submissions[idx], timestamp, false));
    }
}

// End TODO