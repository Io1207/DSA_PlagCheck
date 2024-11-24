#include "plagiarism_checker.hpp"
// You should NOT add ANY other includes to this file.
// Do NOT add "using namespace std;".

// TODO: Implement the methods of the plagiarism_checker_t class

std::vector<long long> getIndexedHashes(const std::vector<int> &vec, const int &window)
{
    const int n = vec.size();
    std::vector<long long> hashes(n - window + 1);

    long long hash = 0;
    long long baseExp = 1; // to remove the first element

    // Compute the hash of the first window
    for (int i = 0; i < window; i++)
    {
        hash = ((hash * base) + vec[i]) % mod;
        baseExp = (baseExp * base) % mod;
    }

    // Compute rolling hashes for all windows
    for (int i = window; i < n; ++i)
    {
        hashes[i - window] = hash;
        hash = ((hash * base) + vec[i]) % mod;
        hash = (hash - ((vec[i - window] * baseExp) % mod) + mod) % mod;
    }

    // Add the last hash
    hashes[n - window] = hash;

    return hashes;
}

// Getting the set of all hashes for all windows of size 'window'
std::unordered_set<long long> getAllHashes(const std::vector<int> &vec, const int &window)
{
    const int n = vec.size();
    std::unordered_set<long long> hashes;

    long long hash = 0;
    long long baseExp = 1; // for removing the first element

    // Compute the hash of the first window
    for (int i = 0; i < window; i++)
    {
        hash = ((hash * base) + vec[i]) % mod;
        baseExp = (baseExp * base) % mod;
    }

    // Compute rolling hashes for all windows
    for (int i = window; i < n; ++i)
    {
        hashes.insert(hash);
        hash = ((hash * base) + vec[i]) % mod;
        hash = (hash - ((vec[i - window] * baseExp) % mod) + mod) % mod;
    }

    // Add the last hash
    hashes.insert(hash);

    return hashes;
}
void totalLengthOfPatternMatches(const int &minLength, std::shared_ptr<tokenized_submission_t> sub1, std::shared_ptr<tokenized_submission_t> sub2)
{
    if (sub1->has_been_flagged && sub2->has_been_flagged)
        return;
    if (!sub1->is_new_submission && !sub2->is_new_submission)
        return;
    
    const auto &submission1 = sub1->tokens;
    const auto &submission2 = sub2->tokens;
    const auto &hashAt1 = sub1->hashVec;
    const auto &hashes2 = sub2->hashSet;
    const int m = submission1.size();

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

    int i = 0;
    int countSmall = 0;
    int countLong = 0;

    while (i < counted1.size())
    {
        if (!counted1[i])
        {
            i++;
            continue;
        }
        else
        {
            int count = 1;
            for (; i < counted1.size(); i++)
            {
                if (counted1[i])
                    count++;
                if (count == 75)
                {
                    if (sub1->is_new_submission)
                    {
                        sub1->flag_plagiarism();
                    }
                    if (sub2->is_new_submission)
                    {
                        sub2->flag_plagiarism();
                    }
                    return;
                }
            }

            // NOTE: change 25 to make better predictions
            if (count >= 25)
                countLong++;
            if (count < 25 && count >= minLength)
                countSmall++;
        }
    }
    auto timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(sub1->timestamp - sub2->timestamp).count();
    if (countSmall >= 10 || countLong >= 7)
    {
        if (timeDiff > 1000)
        {
            sub1->flag_plagiarism();
        }
        else if (timeDiff < -1000)
        {
            sub2->flag_plagiarism();
        }
        else
        {
            sub1->flag_plagiarism();
            sub2->flag_plagiarism();
        }
    }

    // Checking for patchwork plagiarism
    for (const auto &sub : {sub1, sub2})
    {
        if (sub->is_new_submission)
        {
            sub->patch_small = sub->patch_small + countSmall;
            sub->patch_long = sub->patch_long + countLong;
            int length = 150;
            int checking = minLength * sub->patch_small + 25 * sub->patch_long;
            // 3 checks: number of small matches, number of long matches, total length of matches
            if (sub->patch_small >= 20 || sub->patch_long >= 15 || checking > length)
            {
                sub->flag_plagiarism();
            }
        }
    }
    
    return;
}

tokenized_submission_t::tokenized_submission_t(
    std::shared_ptr<submission_t> submission, const std::vector<int> &tokens,
    const std::chrono::time_point<std::chrono::system_clock> &timestamp,
    const std::vector<long long> &hashVec, const std::unordered_set<long long> &hashSet,
    const bool &is_new_submission)
    : submission_t(*submission), tokens(tokens), timestamp(timestamp), hashVec(hashVec),
      hashSet(hashSet), has_been_flagged(false), is_new_submission(is_new_submission)
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
    : num_threads(3), n(0),
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
                totalLengthOfPatternMatches(15, tokenized_submissions[idx], tokenized_submissions[n - 1]);
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

    // Get the hashes for each index as well as the set of all hashes

    const int minLength = 15;
    std::vector<long long>hashVec(n - minLength + 1);
    std::unordered_set<long long>hashSet;

    long long hash = 0;
    long long baseExp = 1; // to remove the first element

    // Compute the hash of the first window
    for (int i = 0; i < minLength; i++)
    {
        hash = ((hash * base) + vec[i]) % mod;
        baseExp = (baseExp * base) % mod;
    }

    // Compute rolling hashes for all windows
    for (int i = minLength; i < n; ++i)
    {
        hashVec[i - minLength] = hash;
        hashSet.insert(hash);
        hash = ((hash * base) + vec[i]) % mod;
        hash = (hash - ((vec[i - minLength] * baseExp) % mod) + mod) % mod;
    }

    // Add the last hash
    hashVec[n - minLength] = hash;
    hashSet.insert(hash);

    return std::make_shared<tokenized_submission_t>(submission, tokens, timestamp, hashVec, hashSet, is_new_submission);
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

// End TODO