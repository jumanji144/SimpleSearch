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