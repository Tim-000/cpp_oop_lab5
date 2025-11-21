#include "forward_list.h"
#include <gtest/gtest.h>
#include <string>
#include <vector>

struct Complex
{
    int a;
    double b;
    std::string s;

    Complex(int aa = 1, double bb = 2.5, std::string ss = "test")
        : a(aa), b(bb), s(std::move(ss)) {}

    bool operator==(const Complex &other) const
    {
        return a == other.a && b == other.b && s == other.s;
    }
};

TEST(ForwardList, PushFrontAndIteration)
{
    FixedPoolResource pool(1024);
    Forward_List<int> lst(&pool);

    lst.push_front(3);
    lst.push_front(2);
    lst.push_front(1);

    EXPECT_EQ(lst.size(), 3u);
    EXPECT_EQ(lst.front(), 1);

    std::vector<int> values;
    for (int x : lst)
        values.push_back(x);

    EXPECT_EQ(values, std::vector<int>({1, 2, 3}));
}

TEST(ForwardList, PopFrontAndClear)
{
    FixedPoolResource pool(1024);
    Forward_List<int> lst(&pool);

    for (int i = 0; i < 10; ++i)
        lst.push_front(i);

    EXPECT_EQ(lst.size(), 10u);

    while (!lst.empty())
        lst.pop_front();

    EXPECT_TRUE(lst.empty());
}

TEST(ForwardList, WorksWithComplexType)
{
    FixedPoolResource pool(1024);
    Forward_List<Complex> lst(&pool);

    lst.emplace_front(10, 3.14, "hello");
    lst.emplace_front(20, 6.28, "world");

    EXPECT_EQ(lst.size(), 2u);

    auto it = lst.begin();
    EXPECT_EQ(it->a, 20);
    EXPECT_EQ(it->s, "world");
    ++it;
    EXPECT_EQ(it->a, 10);
    EXPECT_EQ(it->s, "hello");
}
