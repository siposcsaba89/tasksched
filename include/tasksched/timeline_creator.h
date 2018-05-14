#pragma once
#include <chrono>
#include <string>
#include <map>
#include <list>

/*! \brief Manages timeline? creation for what? task scheduling?
 */
namespace tsch
{
    struct TimePointData
    {
        std::string group;
        std::chrono::time_point<std::chrono::steady_clock> start;                       /*!< Starting time point of what? */.
        std::chrono::time_point<std::chrono::steady_clock> end;                         /*!< End time point of what? */.
    };

    void createTimelineHTML(const std::string & f_name, const std::string & header,     /*!< Creates timeline of what? in HTML */.
        std::chrono::time_point<std::chrono::steady_clock> start_time,                  /*!< Starting time */.
        const std::map<std::string, std::list<tsch::TimePointData>> & data);            /*!< Assigns map data to time points */.
}