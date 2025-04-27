#include "json_parser.h"
#include "cache.h"
#include <iostream>

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        std::cerr << "Используйте: " << argv[0] << " <город отправления> <город прибытия> <дата в формате ГГГГ-ММ-ДД>" << std::endl;
        return 1;
    }

    std::string city_deppart = argv[1];
    std::string arrival = argv[2];
    std::string date_ = argv[3];
    std::string departure_place = city_deppart;
    std::string arrival_place = arrival;
    std::string date = date_;

    std::string cache_dir = "cache";
    cache route_cache(cache_dir);
    json_parser parser(departure_place, arrival_place, date, route_cache);

    parser.find_rout();

    return 0;
}