#include "json_parser.h"
#include <iostream>
#include <algorithm>

json_parser::json_parser(std::string &departure_place, std::string &arrival_place, std::string &date, cache &cache)
    : departure_place_(departure_place), arrival_place_(arrival_place), date_(date), cache_(cache) {}

std::string json_parser::get_station_code(const std::string &city_name)
{
    std::string url = "https://suggests.rasp.yandex.net/all_suggests?format=old&part=" + city_name;
    std::cout << url << std::endl;
    cpr::Response r = cpr::Get(cpr::Url{url});

    if (r.status_code == 200)
    {
        std::cout << nlohmann::json::parse(r.text)[1][0][0];
        return nlohmann::json::parse(r.text)[1][0][0];
    }
    else
    {
        std::cerr << "Ошибка: HTTP-запрос завершился с кодом " << r.status_code << std::endl;
        return "unknown";
    }
}

void json_parser::find_rout()
{
    std::string departure_code = get_station_code(departure_place_);
    std::string arrival_code = get_station_code(arrival_place_);
    if (departure_code == "unknown" || arrival_code == "unknown")
    {
        std::cerr << "Ошибка: Не удалось найти код станции." << std::endl;
        return;
    }
    std::string cache_fname = departure_code + "_" + arrival_code + "_" + date_ + ".json";
    std::string cached_data = cache_.read_cache(cache_fname);

    if (!cached_data.empty())
    {
        print_rout(nlohmann::json::parse(cached_data));
    }
    else
    {
        std::string url = "https://api.rasp.yandex.net/v3.0/search/?apikey=" + api_key +
                          "&format=json&from=" + departure_code +
                          "&to=" + arrival_code +
                          "&lang=ru_RU&page=1&date=" + date_ +
                          "&transfers=true";
        std::cout << url << std::endl;
        cpr::Response r = cpr::Get(cpr::Url{url});

        if (r.status_code == 200)
        {
            nlohmann::json json_data = nlohmann::json::parse(r.text);
            cache_.write_cache(cache_fname, r.text);
            print_rout(json_data);
        }
        else
        {
            std::cerr << "Ошибка: HTTP-запрос завершился с кодом " << r.status_code << std::endl;
        }
    }
}

void json_parser::print_rout(const nlohmann::json &json_data)
{
    if (json_data.contains("segments"))
    {
        const auto &segments = json_data["segments"];

        for (const auto &segment : segments)
        {
            std::vector<Transport> stage;
            if (segment.contains("has_transfers") && segment["has_transfers"].get<bool>())
            {
                for (auto &details : segment["details"])
                {
                    size_t stops = 0;
                    stage.emplace_back(Transport(details));
                    if (stage.back().type == "пересадка")
                        ++stops;
                    if (stops > 2)
                    {
                        stage.clear();
                        break;
                    }
                }
            }
            else
            {
                stage.emplace_back(Transport(segment));
            }
            for (auto &stop : stage)
            {
                std::cout << stop << std::endl;
            }
            std::cout << "------------------------------" << std::endl;
        }
    }
    else
    {
        std::cerr << "Ошибка: Нет данных в ответе API." << std::endl;
    }
}
