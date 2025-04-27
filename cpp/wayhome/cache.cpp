#include "cache.h"

cache::cache(const std::string &cache_dir) : cache_dir_(cache_dir)
{
    if (!std::filesystem::exists(cache_dir_))
    {
        if (!std::filesystem::create_directory(cache_dir_))
        {
            std::cerr << "Error to create cache" << std::endl;
        }
    }
}

std::time_t cache::file_mod_time(const std::filesystem::path &file_path)
{
    auto ftime = std::filesystem::last_write_time(file_path);

    auto sys_time = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());

    return std::chrono::system_clock::to_time_t(sys_time);
}

std::string cache::read_cache(const std::string &filename)
{
    clean_cache(1);
    std::ifstream file(cache_dir_ / filename);
    if (file.is_open())
    {
        std::string data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        return data;
    }
    else
    {
        std::cerr << "File not open to read" << std::endl;
    }
    return "";
}

void cache::write_cache(const std::string &filename, const std::string &data)
{
    if (data.empty())
    {
        std::cerr << "Error write to cache" << std::endl;
        return;
    }
    clean_cache(1);
    std::ofstream file(cache_dir_ / filename);
    if (file.is_open())
    {
        file << data;
        file.close();
    }
    else
    {
        std::cerr << "File not open to write" << std::endl;
    }
}

void cache::clean_cache(int save_hours)
{
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    for (const auto &entry : std::filesystem::directory_iterator(cache_dir_))
    {
        if (entry.is_regular_file())
        {
            auto file_time = file_mod_time(entry.path());
            auto file_age = now_time_t - file_time;
            if (file_age > save_hours * 3600)
            {
                if (!std::filesystem::remove(entry.path()))
                {
                    std::cerr << "Error to clean" << std::endl;
                }
            }
        }
    }
}
