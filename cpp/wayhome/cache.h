#ifndef CACHE_H
#define CACHE_H

#include <string>
#include <filesystem>
#include <chrono>
#include <fstream>
#include <iostream>
#include <ctime>

class cache {
private:
    std::filesystem::path cache_dir_;
    std::chrono::system_clock::time_point last_cleanup;

    std::time_t file_mod_time(const std::filesystem::path &file_path);

public:
    cache(const std::string &cache_dir);

    std::string read_cache(const std::string &filename);

    void write_cache(const std::string &filename, const std::string &data);

    void clean_cache(int save_hours);
};

#endif