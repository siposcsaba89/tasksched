#include "tasksched/tasksched.h"
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
    std::string m_name;
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
        return holder[idx];
    }
    std::vector<std::string> holder;

    virtual ~StringDataHolder() {}
};


int main()
{
    StringDataHolder strholder(10);
    tsch::iomanager iomgr;
    size_t strholder_idx = iomgr.addDataHolder("stringholder" ,strholder);

    std::string & str = iomgr.getDataWritable<StringDataHolder, std::string>(strholder_idx, size_t(1));

    SampleTask T1("T1", 10), T2("T2", 10), T3("T3", 20), T4("T4", 30), T5("T5", 35), T6("T6", 3), T7("T7", 90), T8("T8", 16), T9("T9", 25), T10("T10", 1), T11("T11", 46), T12("T12", 101), T13("T13", 32), T14("T14", 32);
    T1.set_priority(10);
    T2.set_priority(10);
    T3.set_priority(20);
    T4.set_priority(30);
    T5.set_priority(35);
    T6.set_priority(3);
    T7.set_priority(90);
    T8.set_priority(16);
    T9.set_priority(25);
    T10.set_priority(1);
    T11.set_priority(46);
    T12.set_priority(101);
    T13.set_priority(32);
    T14.set_priority(32);

    tsch::threadsched sched(6);
    sched.add_task(&T1);
    sched.add_task(&T2);
    sched.add_task(&T3);
    sched.add_task(&T4);
    sched.add_task(&T5);
    sched.add_task(&T6);
    sched.add_task(&T7);
    sched.add_task(&T8);
    sched.add_task(&T9);
    sched.add_task(&T10);
    sched.add_task(&T11);
    sched.add_task(&T12);
    sched.add_task(&T13);
    sched.add_task(&T14);

    sched.add_dependency(&T1, &T2);
    sched.add_dependency(&T1, &T3);
    sched.add_dependency(&T2, &T5);
    sched.add_dependency(&T2, &T4);
    sched.add_dependency(&T3, &T4);
    sched.add_dependency(&T5, &T6);
    sched.add_dependency(&T4, &T6);
    sched.add_dependency(&T1, &T8);
    sched.add_dependency(&T8, &T9);
    sched.add_dependency(&T8, &T10);
    sched.add_dependency(&T9, &T11);
    sched.add_dependency(&T10, &T11);
    sched.add_dependency(&T1, &T7);
    sched.add_dependency(&T13, &T14);


    sched.start();

    for (int i = 0; i < 100; ++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        //std::cout << "Meg futok: " << i << std::endl;
    }
    sched.finish();
    std::cout << "Lealltam" << std::endl;

    return 0;
}
