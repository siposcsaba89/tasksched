#pragma once
#include <inttypes.h>
#include <mutex>
#include <memory>
#include <thread>
#include <vector>
#include <cassert>
#include <queue>
#include <map>
#include <condition_variable>

struct iomanager
{

};

class task
{
public:
    virtual void execute() = 0;
    virtual void set_iomanager(iomanager * manager) { m_manager = manager; }
    void set_priority(int32_t priority) { m_priority = priority; }
    int32_t priority() { return m_priority; }
private:
    friend class threadsched;
    void set_task_id(int32_t t_id) { m_task_id = t_id; }
    int32_t task_id() { assert(m_task_id != -1); return m_task_id; };
protected:
    iomanager * m_manager = nullptr;
    int32_t m_task_id = -1;
    int32_t m_priority = -1;
};


class threadsched
{
public:
    threadsched(int32_t num_threads);
    void start();
    void finish();
    void add_task(task * t);
    void add_dependency(task *t1, task* t2);
    void add_dependency_by_id(int32_t t1_id, int32_t t2_id);
private:
    void update_queue(task * finsihed_task);
    void clear_task_executed();
    bool all_task_executed() const;
private:
    std::vector<std::thread> m_workers;
    std::vector<task *> m_tasks;
    std::map<int32_t, std::vector<int32_t>> m_deps_ready;
    std::map<int32_t, std::vector<int32_t> > m_forward_deps;
    std::map<int32_t, std::vector<int32_t> > m_backward_deps;
    //std::
    static int32_t s_task_id;
    std::mutex m_task_queue_mutex;
    struct compare_tasks
    {
        bool operator()(task * t1, task * t2) { return t1->priority() < t2->priority(); }
    };
    std::priority_queue < task*, std::vector<task*>, compare_tasks > m_task_queue;
    bool m_stop_execution = false;

    std::mutex m_sched_mutex;
    std::map<int32_t, bool> m_task_executed;
    std::map<int32_t, bool> m_task_scheduled;
    std::condition_variable m_run_workers;
};

