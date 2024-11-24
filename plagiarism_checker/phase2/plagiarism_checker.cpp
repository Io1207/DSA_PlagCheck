#include "plagiarism_checker.hpp"
// You should NOT add ANY other includes to this file.
// Do NOT add "using namespace std;".

// TODO: Implement the methods of the plagiarism_checker_t class

tokenized_submission_t::tokenized_submission_t(
    std::shared_ptr<submission_t> submission, const std::vector<int> &tokens,
    const std::chrono::time_point<std::chrono::system_clock> &timestamp,
    const std::unordered_set<long long> &exact_hashes, const std::unordered_set<long long> &patchwork_hashes, const bool&is_new_submission=false)
    : submission_t(*submission), tokens(tokens), timestamp(timestamp), exact_hashes(exact_hashes),
      patchwork_hashes(patchwork_hashes), patchwork_matches(0), has_been_flagged(false)

{
}

void tokenized_submission_t::flag_plagiarism()
{
    // Don't flag the same submission multiple times
    // NOTE: Maybe we should allow multiple flags for the same submission?
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

plagiarism_checker_t::plagiarism_checker_t(void)
    : num_threads(std::thread::hardware_concurrency()), base(257), mod(1e9 + 7), n(0),
      window_size_exact(75), window_size_patchwork(10), tokenized_submissions(), submission_mutexes(), sub_mutex()
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const int base = 257;
const int mod = 1e9 + 7;
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

//Getting the set of all hashes for all windows of size 'window'
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
void totalLengthOfPatternMatches(const std::vector<int> &submission1, const std::vector<int> &submission2, const int &minLength, tokenized_submission_t* sub1, tokenized_submission_t* sub2)
{
    if(sub1->flagAsPlag && sub2->flagAsPlag) return;
    if(!sub1->is_new_submission && !sub2->is_new_submission) return;
    const int m = submission1.size();

    // make hashAt1, a vector of hashes of size m - 9
    // hashAt1[i] = computeHash(submission1[i ... i + 9])
    auto hashAt1 = getIndexedHashes(submission1, minLength);

    // make hashes2, a set of all the hashes of size 10 in submission2
    auto hashes2 = getAllHashes(submission2, minLength);

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

    int i=0;
    int countSmall=0;
    int countLong=0;

    while(i<counted1.size())
    {
        if(!counted1[i])
        {
            i++; continue;
        }
        else
        {
            int count=1;
            for(;i<counted1.size();i++)
            {
                if (counted1[i]) count++;
                if (count==75){
                    if(sub1->is_new_submission){
                        sub1->flagAsPlag=true;
                    }
                    if(sub2->is_new_submission){
                        sub2->flagAsPlag=true;
                    }
                    return;
                }
                else break;
            }
            
            if (count>=25)  countLong++;
            if (count<25 && count>=10) countSmall++;
        }
    }
    auto timeDiff=std::chrono::duration_cast<std::chrono::milliseconds>(sub1->timestamp - sub2->timestamp).count();
    if(countSmall>=10 || countLong>=6){
        if(timeDiff>1000){
            if(sub1->is_new_submission) sub1->flagAsPlag=true;
        }
        else if(timeDiff<-1000){
            if(sub2->is_new_submission) sub2->flagAsPlag=true;
        }
        else{
            if(sub1->is_new_submission) sub1->flagAsPlag=true;
            if(sub2->is_new_submission) sub2->flagAsPlag=true;

        }
    }
    if (sub1->is_new_submission){
        sub1->patch_small=sub1->patch_small+countSmall;
        sub1->patch_long=sub1->patch_long+countLong;
        int length=150;
        int checking=10*sub1->patch_small+25*sub1->patch_long;
        if(sub1->patch_small>=20 || sub1->patch_long>=10 || checking>length){
            sub1->flagAsPlag=true;
        }
    }
    if (sub2->is_new_submission){
        sub2->patch_small=sub2->patch_small+countSmall;
        sub2->patch_long=sub2->patch_long+countLong;
        int length=150;
        int checking=10*sub2->patch_small+25*sub2->patch_long;
        if(sub2->patch_small>=20 || sub2->patch_long>=10 || checking>length){
            sub2->flagAsPlag=true;
        }
    }


    return;

    
}

plagiarism_checker_t::plagiarism_checker_t(std::vector<std::shared_ptr<submission_t>> __submissions) : plagiarism_checker_t()
{
    // Step 0: Initializations

    auto sub_time = std::chrono::system_clock::now();

    n = __submissions.size();

    std::cerr << "Constructor called, num_threads = " << num_threads << ", n = " << n << std::endl;

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
    std::cerr << "[" << std::this_thread::get_id() << "] Adding submission" << std::endl;

    // Record the submission time
    auto timestamp = std::chrono::system_clock::now();

    tokenized_submissions.push_back(get_tokenized_submission(__submission, timestamp));
    n++;

    int chunk_size = std::ceil((n - 1.0) / num_threads);
    std::vector<std::thread> plag_threads;

    for (int t = 0; t < std::min(num_threads, n - 1); t++)
    {
        int start = t * chunk_size;
        int end = std::min(start + chunk_size, n - 1);
        plag_threads.emplace_back([this, start, end]()
                                  {
            std::cerr << "[" << std::this_thread::get_id() << "] Comparing chunk (" << start << ", " << end << ") with new submission" << std::endl;
            for(int idx = start; idx < end; idx++) {
                check_exact_match(idx, n - 1);
                update_patchwork(idx, n - 1);        
            } });
    }

    for (auto &t : plag_threads)
    {
        t.join();
    }

    std::cerr << "[" << std::this_thread::get_id() << "] Finished adding submission" << std::endl;
}

std::shared_ptr<tokenized_submission_t> plagiarism_checker_t::get_tokenized_submission(
    const std::shared_ptr<submission_t> &submission, const std::chrono::time_point<std::chrono::system_clock> &timestamp)
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

    return std::make_shared<tokenized_submission_t>(submission, tokens, timestamp, exact_hashes, patchwork_hashes);
}

void plagiarism_checker_t::tokenize_hash_chunk(const std::vector<std::shared_ptr<submission_t>> &submissions, const int &start, const int &end, const auto &timestamp)
{
    std::cerr << "[" << std::this_thread::get_id() << "] Tokenizing chunk (" << start << ", " << end << ")" << std::endl;
    for (int idx = start; idx < end; idx++)
    {
        std::lock_guard<std::mutex> lock(sub_mutex);
        tokenized_submissions.push_back(get_tokenized_submission(submissions[idx], timestamp));
    }
}

void plagiarism_checker_t::check_exact_match(const int &i, const int &j)
{
    std::cerr << "[" << std::this_thread::get_id() << "] Checking exact match between (" << i << ", " << j << ")" << std::endl;
    
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
    std::cerr << "[" << std::this_thread::get_id() << "] Updating patchwork between (" << i << ", " << j << ")" << std::endl;

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
    std::cerr << "[" << std::this_thread::get_id() << "] Comparing chunk (" << start_idx << ", " << end_idx << ")" << std::endl;

    // Calculate the pairwise indices and call plagiarism check
    for (int idx = start_idx; idx < end_idx; idx++)
    {
        // Determine the pair (i, j) corresponding to the idx
        // idx = (n - 1) + (n - 2) + ... + (n - i) + (j - i - 1)
        // Manipulate the above equation to get i and j from idx
        int i = (int)(n - 0.5 - std::sqrt(std::pow(n - 0.5, 2) - 2 * idx));
        int j = (int)(i + 1 + (idx - 0.5 * i * (2 * n - i - 1)));

        std::cerr << "[" << std::this_thread::get_id() << "] Comparing (" << i << ", " << j << ")" << std::endl;

        // First, we check for exact matches of length 75 or more
        check_exact_match(i, j);

        // Next, we check for patchwork plagiarism
        // update_patchwork(i, j);
    }
}

// End TODO
