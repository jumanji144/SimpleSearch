#pragma once

#include <thread>
#include <vector>
#include <future>
// check for c++ 20 features
#if __cplusplus >= 202002L
#define likely [[likely]]
#define unlikely [[unlikely]]
#include <filesystem>
namespace fs = std::filesystem;
#else
#define likely
#define unlikely // cannot use likely and unlikely in this case
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif
 
typedef bool (*validation_function)(const fs::directory_entry &);

struct search_context
{
    /**
     * @brief the validation function determines if a file is the correct search target
     */
    validation_function validation;
    std::vector<fs::path> results{};
    bool search_one{false};
    bool only_folders{true};
};

static volatile long long _m_count = 0;
static volatile bool _m_stop_all;

void traverse(fs::directory_entry entry, search_context *ctx)
{
    if (_m_stop_all) unlikely
        return; // this should make all threads return
#ifdef SEARCH_COUNT_FILES
    _m_count++;
#endif
    if (ctx->only_folders)
        if (!fs::is_directory(entry))
            return;
    if (ctx->validation(entry)) unlikely
    {
        // Found, stop this thread and all other threads
        ctx->results.push_back(entry.path());
        if (ctx->search_one)
            _m_stop_all = true;
        return;
    }
    else
    {
        likely
        try
        {
            for (auto &subentry : fs::directory_iterator(entry))
            { // start a normal recursive search
                try
                {
                    traverse(subentry, ctx);
                }
                catch (...)
                {
                    // access denied: skip this folder
                }
            }
        }
        catch (...)
        {
            // access denied: skip this folder
        }
    }
}

void thread_func(std::promise<bool> *p, fs::directory_entry entry, search_context *ctx)
{
    traverse(entry, ctx);
    p->set_value(true);
}

/**
 * @brief start the deep backwards traverse search using a defined search configuration
 * @param start the path to start the search from
 * @param ctx the search configuration
 * @param maxDepth the maximum depth to search
 * @return whether the search was successful or not
 */
int start_deep_traverse_search(fs::path start, search_context *ctx, int maxDepth = 2)
{
    std::vector<std::promise<bool> *> promises;
    fs::path last = start;
    int depth = 0;
    while (depth <= maxDepth)
    {
        // get a iterator of the current directory
        for (const auto &entry : fs::directory_iterator(start))
        {
            if (entry.path() == last)
                continue;
            // create a promise for each thread
            std::promise<bool> *p = new std::promise<bool>();
            promises.push_back(p);
            // create a thread for each promise
            std::thread t(thread_func, p, entry, ctx);
            // start the thread
            t.detach();
        }

        // wait for all threads to finish
        for (auto &p : promises)
        {
            p->get_future().wait();
        }

        // clear for next iteration
        promises.clear();
        last = start;

        fs::path parent = start.parent_path();
        // try to go to the parent directory
        if (parent == start || parent.empty())
            break;
        start = parent;

        depth++; // we are going deeper
    }

    return 0;
}