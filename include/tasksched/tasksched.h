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
#include <list>
#include <tasksched/timeline_creator.h>
#include <functional>

/*! \brief Manages scheduling tasks between application thread pools
 */
namespace tsch
{
    /*! Implements common interface between data holders
     */
    struct DataHolder
    {
        virtual void swapBuffers() = 0;                                         /*!< Thread safe function to swap data buffers, drops the oldest package and loads the newer one into the queue*/
        virtual ~DataHolder() {}
    };

    /*! Manages input/output data queues between tasks. Every task has 0 or more input and 0 or more output DataHolder. DataHolders can store 1 or more consecutive frame data.
     */
    struct iomanager
    {
        std::vector<DataHolder*> data_holders;                                  /*!< Data buffer */                                                                           
        std::map<std::string, size_t> data_holders_indices;                     /*!< Indexed map for data buffer */                                                                     

        template<typename HolderType, typename RetDataType>                                                                     /*! Retrieves writable data for stream index, data index, and device index*/
        RetDataType & getDataWritable(size_t stream_idx, size_t data_idx, size_t device_idx)                                    
        {
            return dynamic_cast<HolderType&>(*data_holders[stream_idx]).getElement(data_idx, device_idx);
        }
        
        template<typename HolderType, typename RetDataType>                                                                     /*! Retrieves writable data for stream name, data index, and device index */
        RetDataType & getDataWritable(const std::string & stream_name, size_t data_idx, size_t device_idx)
        {
            return dynamic_cast<HolderType&>(*data_holders[data_holders_indices[stream_name]]).getElement(data_idx, device_idx);
        }

        template<typename HolderType, typename RetDataType>                                                                     /*! Retrieves readable data for stream index, data index, and device index */
        const RetDataType & getData(size_t stream_idx, size_t data_idx, size_t device_idx) const
        {
            return dynamic_cast<HolderType&>(*data_holders[stream_idx]).getElement(data_idx, device_idx);
        }

        template<typename HolderType, typename RetDataType>                                                                     /*! Retrieves readable data for stream name, data index, and device index */
        const RetDataType & getData(const std::string & stream_name, size_t data_idx, size_t device_idx) const
        {
            auto  it = data_holders_indices.find(stream_name);
            assert(it != data_holders_indices.end());
            return dynamic_cast<HolderType&>(*data_holders[it->second]).getElement(data_idx, device_idx);
        }
        
        /*! Adds new data to data holder */
        size_t addDataHolder(const std::string & name, DataHolder & dh)                                                         
        {
            size_t curr_num = data_holders.size();                                                                              
            data_holders.push_back(&dh);
            data_holders_indices[name] = curr_num;
            return curr_num;
        }

        void swapBuffers()
        {
            for (auto & dh : data_holders)
                dh->swapBuffers();
        }

    };

    /*! General task interface
     */
    class task
    {
    public:
        virtual void execute() = 0;
        virtual void set_iomanager(iomanager * manager) { m_io_manager = manager; }                                             /*!< Sets IO manager */
        virtual void set_name(const std::string & name) { m_name = name; }                                                      /*!< Sets IO manager name */
        const std::string & get_name() const { return m_name; }                                                                 /*!< Sets task name */
        void set_priority(int32_t priority) { m_priority = priority; }                                                          /*!< Sets task priority */
        int32_t priority() { return m_priority; }                                                                               /*!< Returns task priority */
    private:
        friend class threadsched;
        void set_task_id(int32_t t_id) { m_task_id = t_id; }
        int32_t task_id() { assert(m_task_id != -1); return m_task_id; };
    protected:
        iomanager * m_io_manager = nullptr;
        int32_t m_task_id = -1;
        int32_t m_priority = -1;
        std::string m_name;
    };

    /*! Task scheduler
     */
    class threadsched
    {
    public:
        threadsched(int32_t num_threads, std::function<bool(void)> all_task_executed_callback);
        void start();                                                                                                       /*!< Starts scheduler */
        void finish();                                                                                                      /*!< Finishes scheduler */
        void add_task(task * t);                                                                                            /*!< Adds tasks to scheduler */
        void add_dependency(task *t1, task* t2);                                                                            /*!< Defines dependencies between tasks (t2 must wait until t1 executed) */
        void add_dependency_by_id(int32_t t1_id, int32_t t2_id);                                                            /*!< Defines dependencies between tasks by their ID */
        std::chrono::time_point<std::chrono::steady_clock> get_start_time() const;                                          /*!< Returns start time of the scheduler */
        const std::map<std::string, std::list<TimePointData>> & getTimlineData() const;                                     /*!< Returns start and end time of all executed tasks */
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
        
        std::function<bool(void)> m_all_task_executed_callback;

        std::map<std::string, std::list<TimePointData>> m_timestamps;
        std::chrono::time_point<std::chrono::steady_clock> m_start_time;
    };

}