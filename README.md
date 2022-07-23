## Simple File Search C++
A simple one file library to easily and fastly search for files in the entire filesystem.

### Usage
The library is better optimized for C++20 but will compile on earlier versions.    
A example is given in the [example.cpp](example.cpp) file.
```cpp
#include "folder_search.hpp"

int main() {
    search_context* ctx = new search_context; // the object HAS to be heap allocated because it is passed to the thread function
    ctx->only_folders = false;
    ctx->validation = [](const fs::directory_entry& entry) {
        return entry.path().extension() == ".cpp";
    };
    ctx->search_one = true;
    start_deep_traverse_search(fs::current_path(), ctx);
}
```

The library works in the way that it backward searches the filesystem from where it originates and then uses multiple threads to traverse the filesystem
to create a tree like structure along the way

the search is controlled via the `search_context` struct (which must be heap allocated to be used).
```cpp
struct search_context
{
    validation_function validation; // function that checks if a entry is the one to be searched for
    std::vector<fs::path> results{}; // a result vector to store the results in
    bool search_one{false}; // if true, only one result is searched for and the search is stopped after the first result is found
    bool only_folders{true}; // if true, only folders are searched for, otherwise files are searched for (this will speed up the search)
};
```

### Safety
The code is realtivily stable but there is no thread limit so be careful when using it, it gives a huge performance boost if you use it correctly. But can have unexpected results.

