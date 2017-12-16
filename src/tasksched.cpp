#include "tasksched/tasksched.h"
#include <inttypes.h>
#include <mutex>
#include <memory>
#include <thread>
#include <vector>
#include <cassert>
#include <queue>
#include <map>
#include <iostream>

namespace tsch
{

    int32_t threadsched::s_task_id = 0;

    threadsched::threadsched(int32_t num_threads, std::function<void(void)> all_task_executed_callback)
    {
        m_all_task_executed_callback = all_task_executed_callback;
        m_workers.resize(num_threads);
    }

    void threadsched::start()
    {
        m_start_time = std::chrono::high_resolution_clock::now();
        clear_task_executed();
        update_queue(nullptr);
        auto a = [&](int th_id)
        {
            while (!m_stop_execution)
            {
                task * to_execute = nullptr;
                {
                    std::unique_lock<std::mutex> queue_lock(m_task_queue_mutex);
                    if (!m_task_queue.empty())
                    {
                        to_execute = m_task_queue.top();
                        m_task_queue.pop();
                    }
                    else
                    {
                        m_run_workers.wait(queue_lock);
                    }
                }
                if (to_execute != nullptr)
                {
                    m_deps_ready[to_execute->task_id()].resize(0);
                    
                    TimePointData t;
                    t.start = std::chrono::high_resolution_clock::now();
                    to_execute->execute();
                    t.end = std::chrono::high_resolution_clock::now();
                    t.group = std::to_string(th_id);
                    m_timestamps[to_execute->get_name()].emplace_back(t);
                    printf("Task executed: %s \n", to_execute->get_name().c_str());
                    update_queue(to_execute);

                }
            }
        };

        int th_id = 0;
        for (auto & th : m_workers)
        {
            th = std::thread(a, th_id);
            ++th_id;
        }
    }

    void threadsched::finish()
    {
        m_stop_execution = true;
        for (auto & w : m_workers)
            if (w.joinable())
                w.join();
    }

    void threadsched::add_task(task * t)
    {
        t->set_task_id(s_task_id++);
        m_tasks.push_back(t);
    }

    void threadsched::add_dependency(task * t1, task * t2)
    {
        add_dependency_by_id(t1->task_id(), t2->task_id());
    }

    void threadsched::add_dependency_by_id(int32_t t1_id, int32_t t2_id)
    {
        m_forward_deps[t1_id].push_back(t2_id);
        m_backward_deps[t2_id].push_back(t1_id);
    }

    std::chrono::time_point<std::chrono::steady_clock> threadsched::get_start_time() const
    {
        return m_start_time;
    }

    const std::map<std::string, std::list<TimePointData>>& threadsched::getTimlineData() const
    {
        return m_timestamps;
    }

    void threadsched::update_queue(task * finshed_task)
    {
        std::lock_guard<std::mutex> lock(m_sched_mutex);
        if (finshed_task != nullptr)
            m_task_executed[finshed_task->task_id()] = true;

        if (all_task_executed())
        {
            std::cout << "Reschedule \n" << std::endl;
            clear_task_executed();
            m_all_task_executed_callback();
        }


        if (finshed_task != nullptr)
        {
            for (auto & dep : m_forward_deps[finshed_task->task_id()])
                m_deps_ready[dep].push_back(finshed_task->task_id());
        }

        //schedule task
        for (auto & t : m_tasks)
        {
            if (!m_task_executed[t->task_id()] && !m_task_scheduled[t->task_id()])
            {
                bool all_deps_ready = true;
                for (auto & t_dep : m_backward_deps[t->task_id()])
                {
                    all_deps_ready = all_deps_ready && m_task_executed[t_dep];
                    if (!all_deps_ready)
                        break;
                }
                if (all_deps_ready)
                {
                    {
                        std::lock_guard<std::mutex> lock(m_task_queue_mutex);
                        m_task_scheduled[t->task_id()] = true;
                        m_task_queue.push(t);
                    }
                    m_run_workers.notify_all();
                }
            }
        }
    }

    void threadsched::clear_task_executed()
    {
        for (auto & m : m_tasks)
        {
            m_task_executed[m->task_id()] = false;
            m_task_scheduled[m->task_id()] = false;
        }
        static auto start = std::chrono::steady_clock::now();
        auto end = std::chrono::steady_clock::now();
        std::cout << "Elapsed time in milliseconds : "
            << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
            << " ms" << std::endl;
        start = end;

    }

    bool threadsched::all_task_executed() const
    {
        for (auto & t : m_task_executed)
            if (!t.second) return false;
        return true;
    }
}