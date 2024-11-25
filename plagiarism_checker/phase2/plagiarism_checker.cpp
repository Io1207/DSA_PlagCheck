#include "plagiarism_checker.hpp"
// You should NOT add ANY other includes to this file.
// Do NOT add "using namespace std;".

// TODO: Implement the methods of the plagiarism_checker_t class

// Function to check for pattern matches and flag plagiarism based on tokenized submissions
void totalLengthOfPatternMatches(const int &minLength, std::shared_ptr<tokenized_submission_t> sub1, std::shared_ptr<tokenized_submission_t> sub2)
{
    {
        std::lock_guard<std::mutex> lock1(sub1->flag_mutex);
        std::lock_guard<std::mutex> lock2(sub2->flag_mutex);
        
        // If both submissions are flagged, skip the comparison
        if (sub1->has_been_flagged && sub2->has_been_flagged)
            return;
    }
    // If both submissions are not new, skip plagiarism check
    if (!sub1->is_new_submission && !sub2->is_new_submission)
        return;

    // Token and hash data of the submissions
    const std::vector<int> &submission1 = sub1->tokens;
    const std::vector<int> &submission2 = sub2->tokens;
    const std::vector<long long> &hashAt1 = sub1->hashVec;
    const std::unordered_set<long long> &hashes2 = sub2->hashSet;
    const int m = submission1.size();

    // If the submissions are identical, check time difference and flag accordingly
    if (submission1==submission2)
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

    // To track which tokens have been counted in submission1
    std::vector<bool> counted1(m, false);

    // Compare tokens and check if hashes match
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

    // Track counts for short and long matches
    int countSmall = 0;
    int countLong = 0;
    int count = 0;

    // Iterate through tokens to check for pattern matches
    for(int i = 0; i < m; i++) 
    {
        if(counted1[i]) 
        {
            count++;
        } 
        else 
        {
            //if (count<75) continue; //in case threshold hasn't been crossed go to the next iteration
            
            // If a match is found, check conditions for plagiarism flagging
            if(count >= std::min(m,75)) 
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
                    return;
                }   
            } 
            else if(count >= 25) 
            {
                countLong++;
            } 
            else if(count >= minLength) 
            {
                countSmall++;
            }
            count = 0;
        }
    }

     // Compare time difference and flag plagiarism based on count thresholds
    auto timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(sub1->timestamp - sub2->timestamp).count();
    if(countSmall + countLong >= 10 || countLong >= 6 || 10*countSmall + 25*countLong >= 150)
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
            int length = 275;
            int checking = minLength * sub->patch_small + 25 * sub->patch_long;
            // 3 checks: number of small matches, number of long matches, total length of matches
            // if (sub->patch_small >= 20 || sub->patch_long >= 15 || checking > length)
            if(sub->patch_small + sub->patch_long >= 20 || checking >= length || sub->patch_long >= 11)
            {
                // std::cerr << "patch_small >= 20" << std::endl;
                sub->flag_plagiarism();
            }
        }
    }

    return;
}

// Constructor for tokenized_submission_t to initialize necessary fields
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

// Method to flag a submission for plagiarism
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

// Constructor for waiting_submission_t
waiting_submission_t::waiting_submission_t(std::shared_ptr<submission_t> submission,
                                           const std::chrono::time_point<std::chrono::steady_clock> &timestamp,
                                           const bool &is_new_submission)
    : submission_t(*submission), timestamp(timestamp), is_new_submission(is_new_submission)
{
}


plagiarism_checker_t::plagiarism_checker_t(void)
    : n(0), tokenized_submissions(), sub_mutex(), terminate_thread(false)
{
    // for (int t = 0; t < num_threads; t++)
    // {
    //     worker_threads.emplace_back([this]()
    //                                 { this->process_submissions(); }); // ???
    // }
    worker_thread = std::thread([this](){ this->process_submissions(); });
}

// class constructor
plagiarism_checker_t::plagiarism_checker_t(std::vector<std::shared_ptr<submission_t>> __submissions) : plagiarism_checker_t()
{
    // Record timestamp
    auto timestamp = std::chrono::steady_clock::now();
    
    // std::cerr << "Constructor timestamp: " << timestamp.time_since_epoch().count() << std::endl;

    {
        std::lock_guard<std::mutex> lock(sub_mutex);

        for (const auto &submission : __submissions)
        {
            auto waiting_submission = std::make_shared<waiting_submission_t>(submission, timestamp, false);
            process_queue.push(waiting_submission);
        }
    }
    queue_cv.notify_all();
}

//class destructor 
plagiarism_checker_t::~plagiarism_checker_t(void)
{
    {
        std::lock_guard<std::mutex> lock(sub_mutex);
        terminate_thread = true;
    }

    queue_cv.notify_all();

    worker_thread.join();
}


void plagiarism_checker_t::add_submission(std::shared_ptr<submission_t> __submission)
{
    // Record the submission time
    auto timestamp = std::chrono::steady_clock::now();
    auto waiting_submission = std::make_shared<waiting_submission_t>(__submission, timestamp, true);

    std::cerr << "Adding submission with timestamp: " << timestamp.time_since_epoch().count() << std::endl;

    {
        std::lock_guard<std::mutex> lock(sub_mutex);
        process_queue.push(waiting_submission);
    }

    queue_cv.notify_one();
}

//function to handle the proccesing of submissions tokenization and checks 
void plagiarism_checker_t::process_submissions()
{
    while (true)
    {
        std::shared_ptr<waiting_submission_t> submission_to_process;
        {
            std::unique_lock<std::mutex> lock(sub_mutex);
            queue_cv.wait(lock, [this]
                          { return !process_queue.empty() || terminate_thread; });

            if (terminate_thread && process_queue.empty())
            {
                return;
            }

            submission_to_process = process_queue.front();
            process_queue.pop();
        }

        // Process the submission
        auto tokenized_submission = get_tokenized_submission(
            submission_to_process, submission_to_process->timestamp, submission_to_process->is_new_submission
        );

        // Add the submission to the list of tokenized submissions
        {
            std::lock_guard<std::mutex> lock(sub_mutex);
            tokenized_submissions.push_back(tokenized_submission);
            n++;
        }

        // Perform plagiarism check using a single thread
        for (int idx = 0; idx < n - 1; idx++)
        {
            totalLengthOfPatternMatches(13, tokenized_submissions[idx], tokenized_submission);
        }
    }
}

// Method to tokenize a submission and create tokenized_submission_t object
std::shared_ptr<tokenized_submission_t> plagiarism_checker_t::get_tokenized_submission(
    const std::shared_ptr<submission_t> &submission, const std::chrono::time_point<std::chrono::steady_clock> &timestamp,
    const bool &is_new_submission)
{
    std::cerr << "[" << std::this_thread::get_id() << "] Preprocessing submission" << std::endl;

    // Assuming required mutexes are locked before calling this function

    // Tokenize the submission
    tokenizer_t tkzr(submission->codefile); // ???
    auto tokens = tkzr.get_tokens();
    const int length = tokens.size();

    // Get the hashes for each index as well as the set of all hashes

    const int minLength = 13;
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

// End TODO
