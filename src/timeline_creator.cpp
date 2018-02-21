#include "tasksched/timeline_creator.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip> // put_time

void tsch::createTimelineHTML(const std::string & f_name,
    const std::string & header,
    std::chrono::time_point<std::chrono::steady_clock> start_time,
    const std::map<std::string, std::list<tsch::TimePointData>> & data)
{

    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d-%H-%M-%S");
    
    std::ofstream tlf(ss.str() + "_" + f_name);
    if (!tlf.is_open())
        return;
    tlf <<
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head>\n"
        "<style>\n"
        "table {\n"
        "    font-family: arial, sans-serif;\n"
        "    border-collapse: collapse;\n"
        "    width: 100%;\n"
        "}\n"
        "td, th {\n"
        "    border: 1px solid #dddddd;\n"
        "    text-align: left;\n"
        "    padding: 8px;\n"
        "}\n"
        "tr:nth-child(even) {\n"
        "    background-color: #dddddd;\n"
        "}\n"
        "</style>\n"
        "</head>\n"
        "<body>\n"
        "<h1>" + header + "</h1>\n"
        "<script type = \"text/javascript\" src = \"https://www.gstatic.com/charts/loader.js\"></script> \n"
        "<script type = \"text/javascript\">\n"
        "    google.charts.load(\"current\", { packages:[\"timeline\"] });\n"
        "    google.charts.setOnLoadCallback(drawChart);\n"
        "        function drawChart() {\n"

        "            var container = document.getElementById('example5.2');\n"
        "            var chart = new google.visualization.Timeline(container);\n"
        "            var dataTable = new google.visualization.DataTable();\n"

        "            dataTable.addColumn({ type: 'string', id : 'Thread' });\n"
        "            dataTable.addColumn({ type: 'string', id : 'Task Name' });\n"
        "            dataTable.addColumn({ type: 'date', id : 'Start' });\n"
        "            dataTable.addColumn({ type: 'date', id : 'End' });\n"
        "            dataTable.addRows([\n";

    int64_t max = 0;
    int64_t height = 600;
    int scale = 3;
    struct TimeStat
    {
        int64_t max_duration = 0;
        int64_t min_duration = std::numeric_limits<int64_t>::max();
        double avg_duration = 0;
    };
    std::map<std::string, TimeStat> time_stats;
    std::map<std::string, int> groupe_id_count;
    for (auto & d : data)
    {
        TimeStat stat;
        auto it = d.second.begin();
        if (d.second.size() > 50)
            std::advance(it, 50);
        else
            continue;

        for (; it != d.second.end(); ++it)
        {
            auto & dd = *it;
            std::chrono::milliseconds start = std::chrono::duration_cast<std::chrono::milliseconds>( dd.start - start_time);
            std::chrono::milliseconds end = std::chrono::duration_cast<std::chrono::milliseconds>(dd.end - start_time);
            int64_t dur = (end - start).count();
            if (stat.max_duration < dur)
                stat.max_duration = dur;
            if (stat.min_duration > dur)
                stat.min_duration = dur;
            stat.avg_duration += dur;

            tlf << "                ['" + dd.group + "', '" + d.first + "', new Date(" + std::to_string(start.count()) + "), new Date(" + std::to_string(end.count()) + ")], \n";
            if (max < end.count())
                max = end.count();
            ++groupe_id_count[dd.group];
        }
        stat.avg_duration /= (d.second.size() - 50);
        time_stats[d.first] = std::move(stat);
    }

    tlf << "        ]);\n";

    tlf <<
        "        var options = {\n"
        "            width: " + std::to_string(max * scale) + ",\n"
        "            height: " + std::to_string(groupe_id_count.size() * 50) + "\n"
        "        };\n"

        "        chart.draw(dataTable, options);\n"
        "    }\n"
        "</script>\n"
        "<div id=\"example5.2\" \"></div>\n"
        "<table>\n"
        "  <tr>\n";

    tlf << 
        "    <th>Name</th>\n"
        "    <th>Min duration</th>\n"
        "    <th>Max duration</th>\n"
        "    <th>Average duration</th>\n"
        "  </tr>\n";
    for (auto & st : time_stats)
    {
        tlf << 
            "  <tr>\n"
            "    <td>" + st.first + "</td>\n"
            "    <td>" + std::to_string(st.second.min_duration) + "</td>\n"
            "    <td>" + std::to_string(st.second.max_duration) + "</td>\n"
            "    <td>" + std::to_string(st.second.avg_duration) + "</td>\n"
            "  </tr>\n";
    }

    tlf <<
        "</table>\n"
        "</body>\n"
        "</html>\n";
    tlf.close();
}
