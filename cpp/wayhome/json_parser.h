#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include <string>
#include <nlohmann/json.hpp>
#include <cpr/cpr.h>
#include "cache.h"

class json_parser
{
private:
    std::string api_key = "20ab99c0-22c4-4e49-bec5-9fb0d9f5b614";
    std::string departure_place_;
    std::string arrival_place_;
    std::string date_;
    cache &cache_;

public:
    struct Transport
    {
        Transport() = default;

        Transport(const nlohmann::json &json)
        {
            if (json.contains("is_transfer") && json["is_transfer"].get<bool>())
            {
                from_city_name = json["transfer_from"].value("title", "unknown");
                to_city_name = json["transfer_to"].value("title", "unknown");
                type = "Пересадка";
                uid = "";
            }
            else
            {
                time_departure = json.value("departure", "unknown");
                time_arrival = json.value("arrival", "unknown");
                from_city_name = json["from"].value("title", "unknown");
                to_city_name = json["to"].value("title", "unknown");
                type = json["thread"].value("transport_type", "unknown");
                uid = json["thread"].value("number", "unknown");
            }
        }

        friend std::ostream &operator<<(std::ostream &os, const Transport &transport)
        {
            os << transport.from_city_name << " -- "
               << transport.type << " "
               << transport.uid << " --> "
               << transport.to_city_name
               << " (" << transport.time_arrival << " : " << transport.time_departure << ")";
            return os;
        }

        operator nlohmann::json() const
        {
            return nlohmann::json{
                {"time_arrival", time_arrival},
                {"time_departure", time_departure},
                {"type", type},
                {"uid", uid},
                {"from_city_name", from_city_name},
                {"to_city_name", to_city_name}};
        }

        std::string time_arrival;
        std::string time_departure;
        std::string type;
        std::string uid;
        std::string from_city_name;
        std::string to_city_name;
    };
    json_parser(std::string &departure_place, std::string &arrival_place, std::string &date, cache &cache);
    std::string get_station_code(const std::string &city_name);
    void find_rout();
    void print_rout(const nlohmann::json &json_data);
};

#endif