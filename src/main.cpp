#include <iostream>
#include <string>

#include "../include/memory_resource.h"
#include "../include/forward_list.h"

struct Data
{
    int num;
    std::string name;
};

int main()
{
    Forward_List<int> list1;
    list1.push_front(3);
    list1.push_front(2);
    list1.push_front(1);

    std::cout << "Integers: ";
    for (auto it = list1.begin(); it != list1.end(); ++it)
    {
        std::cout << *it << " ";
    }
    std::cout << std::endl;

    Forward_List<Data> list2;
    list2.push_front({100, "data1"});
    list2.push_front({200, "data2"});
    list2.push_front({300, "data3"});

    std::cout << "Structures: ";
    for (const auto &item : list2)
    {
        std::cout << "{" << item.num << ", \"" << item.name << "\"} ";
    }
    std::cout << std::endl;

    FixedPoolResource mr(1024);
    void *p1 = mr.allocate(64, alignof(std::max_align_t));
    void *p2 = mr.allocate(128, alignof(std::max_align_t));
    mr.deallocate(p1, 64, alignof(std::max_align_t));
    mr.deallocate(p2, 128, alignof(std::max_align_t));

    void *p3 = mr.allocate(64, alignof(std::max_align_t));
    mr.deallocate(p3, 64, alignof(std::max_align_t));

    FixedPoolResource custom_mr(2048);
    Forward_List<int> list3(&custom_mr);
    for (int i = 0; i < 5; ++i)
    {
        list3.push_front(i * 10);
    }

    std::cout << "Custom allocator list: ";
    for (const auto &item : list3)
    {
        std::cout << item << " ";
    }
    std::cout << std::endl;

    std::cout << "Front element: " << list2.front().num << std::endl;
    list2.pop_front();
    std::cout << "After pop_front: " << list2.front().num << std::endl;
    std::cout << "List size: " << list2.size() << std::endl;
    std::cout << "Is empty: " << (list2.empty() ? "yes" : "no") << std::endl;

    return 0;
}
