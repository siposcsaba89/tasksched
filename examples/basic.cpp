#include "tasksched/tasksched.h"
#include "tasksched/timeline_creator.h"
#include <string>
#include <iostream>



class SampleTask : public tsch::task
{
public:
    SampleTask(const std::string & name, int sleep_time) { m_name = name; m_sleep_time = sleep_time; }
    virtual void execute() override
    {
        std::cout << m_name << ": " << m_task_id << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(m_sleep_time));
    }
    int m_sleep_time;
};


struct StringDataHolder : public tsch::DataHolder
{
    StringDataHolder(size_t h_size)
    {
        for (size_t i = 0; i < h_size; ++i)
            holder.push_back(std::to_string(i));
    }
    std::string & getElement(size_t idx)
    {
        size_t s = holder.size();
        return holder[(idx + s - m_idx_counter) % s];
    }
    std::vector<std::string> holder;


    void swapBuffers()
    {
        m_idx_counter = (m_idx_counter + 1) % holder.size();
    }
    size_t m_idx_counter = 0;
    virtual ~StringDataHolder() {}
};


int main()
{
    StringDataHolder strholder(10);

    for (int i = 0; i < 25; ++i)
    {
        std::cout << strholder.getElement(1) << std::endl;
        strholder.swapBuffers();
    }

    tsch::iomanager iomgr;
    size_t strholder_idx = iomgr.addDataHolder("stringholder" ,strholder);

    std::string & str = iomgr.getDataWritable<StringDataHolder, std::string>(strholder_idx, size_t(1));

    std::unique_ptr<SampleTask> T1(std::make_unique< SampleTask>("T1", 10)),
        T2(std::make_unique< SampleTask>("T2", 10)),
        T3(std::make_unique< SampleTask>("T3", 20)),
        T4(std::make_unique< SampleTask>("T4", 30)),
        T5(std::make_unique< SampleTask>("T5", 35)),
        T6(std::make_unique< SampleTask>("T6", 3)),
        T7(std::make_unique< SampleTask>("T7", 90)),
        T8(std::make_unique< SampleTask>("T8", 16)),
        T9(std::make_unique< SampleTask>("T9", 25)),
        T10(std::make_unique< SampleTask>("T10", 1)),
        T11(std::make_unique< SampleTask>("T11", 46)),
        T12(std::make_unique< SampleTask>("T12", 101)),
        T13(std::make_unique< SampleTask>("T13", 32)),
        T14(std::make_unique< SampleTask>("T14", 32));
    T1->set_priority(10);
    T2->set_priority(10);
    T3->set_priority(20);
    T4->set_priority(30);
    T5->set_priority(35);
    T6->set_priority(3);
    T7->set_priority(90);
    T8->set_priority(16);
    T9->set_priority(25);
    T10->set_priority(1);
    T11->set_priority(46);
    T12->set_priority(101);
    T13->set_priority(32);
    T14->set_priority(32);
    tsch::threadsched sched(6, []() {printf("All task executed, starting again!"); return true; });
    sched.add_task(std::move(T1));
    sched.add_task(std::move(T2));
    sched.add_task(std::move(T3));
    sched.add_task(std::move(T4));
    sched.add_task(std::move(T5));
    sched.add_task(std::move(T6));
    sched.add_task(std::move(T7));
    sched.add_task(std::move(T8));
    sched.add_task(std::move(T9));
    sched.add_task(std::move(T10));
    sched.add_task(std::move(T11));
    sched.add_task(std::move(T12));
    sched.add_task(std::move(T13));
    sched.add_task(std::move(T14));

    sched.add_dependency(T1.get(), T2.get());
    sched.add_dependency(T1.get(), T3.get());
    sched.add_dependency(T2.get(), T5.get());
    sched.add_dependency(T2.get(), T4.get());
    sched.add_dependency(T3.get(), T4.get());
    sched.add_dependency(T5.get(), T6.get());
    sched.add_dependency(T4.get(), T6.get());
    sched.add_dependency(T1.get(), T8.get());
    sched.add_dependency(T8.get(), T9.get());
    sched.add_dependency(T8.get(), T10.get());
    sched.add_dependency(T9.get(), T11.get());
    sched.add_dependency(T10.get(), T11.get());
    sched.add_dependency(T1.get(), T7.get());
    sched.add_dependency(T13.get(), T14.get());


    sched.start();

    for (int i = 0; i < 100; ++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        //std::cout << "Meg futok: " << i << std::endl;
    }
    sched.finish();
    std::cout << "Lealltam" << std::endl;
    tsch::createTimelineHTML("./", "timeline.html", "Tasksched example timeline", sched.get_start_time(), sched.getTimlineData());
    return 0;
}
