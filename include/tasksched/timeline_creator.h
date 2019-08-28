#pragma once
#include <chrono>
#include <string>
#include <map>
#include <list>

namespace tsch
{
    struct TimePointData
    {
        std::string group;
        std::chrono::time_point<std::chrono::steady_clock> start;
        std::chrono::time_point<std::chrono::steady_clock> end;
    };

    void createTimelineHTML(const std::string & p_name, const std::string & f_name, const std::string & header,
        std::chrono::time_point<std::chrono::steady_clock> start_time,
        const std::map<std::string, std::list<tsch::TimePointData>> & data);
}