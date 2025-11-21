#include <gtest/gtest.h>
#include "../include/forward_list.h"
#include "../include/memory_resource.h"

#include <string>
#include <vector>
#include <sstream>

class CerrRedirect
{
public:
    CerrRedirect(std::stringstream &capture)
        : old_buf(std::cerr.rdbuf(capture.rdbuf())) {}
    ~CerrRedirect() { std::cerr.rdbuf(old_buf); }

private:
    std::streambuf *old_buf;
};

TEST(FixedPoolResource, AllocatesAndDeallocatesWithReuse)
{
    FixedPoolResource pool(1024);

    void *p1 = pool.allocate(100, 8);
    void *p2 = pool.allocate(100, 8);

    pool.deallocate(p1, 100, 8);

    void *p3 = pool.allocate(100, 8);
    EXPECT_EQ(p1, p3);

    pool.deallocate(p2, 100, 8);
    pool.deallocate(p3, 100, 8);
}

TEST(FixedPoolResource, CoalescesAdjacentBlocks)
{
    FixedPoolResource pool(1024);

    void *p1 = pool.allocate(100, 1);
    void *p2 = pool.allocate(100, 1);
    void *p3 = pool.allocate(100, 1);

    pool.deallocate(p1, 100, 1);
    pool.deallocate(p3, 100, 1);
    pool.deallocate(p2, 100, 1);

    void *big = pool.allocate(300, 1);
    EXPECT_NE(big, nullptr);

    pool.deallocate(big, 300, 1);
}

TEST(PmrForwardList, DefaultConstructorUsesNewDeleteResource)
{
    Forward_List<int> lst;
    lst.emplace_front(42);
    EXPECT_EQ(lst.front(), 42);
    EXPECT_EQ(lst.size(), 1u);
}

TEST(PmrForwardList, PushFrontAndIterationSimpleType)
{
    FixedPoolResource pool(10 * 1024);
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

TEST(PmrForwardList, PopFrontAndClear)
{
    FixedPoolResource pool(10 * 1024);
    Forward_List<int> lst(&pool);

    for (int i = 0; i < 100; ++i)
        lst.emplace_front(i);

    EXPECT_EQ(lst.size(), 100u);

    while (!lst.empty())
        lst.pop_front();

    EXPECT_TRUE(lst.empty());
    EXPECT_EQ(lst.size(), 0u);
}

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

TEST(PmrForwardList, WorksWithNonTrivialType)
{
    FixedPoolResource pool(10 * 1024);
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

TEST(PmrForwardList, IteratorMeetsForwardIteratorRequirements)
{
    FixedPoolResource pool(1024);
    Forward_List<int> lst(&pool);
    lst.emplace_front(1);
    lst.emplace_front(2);

    using Iter = Forward_List<int>::iterator;
    static_assert(std::is_same_v<typename std::iterator_traits<Iter>::iterator_category,
                                 std::forward_iterator_tag>);

    Iter it = lst.begin();
    EXPECT_EQ(*it, 2);
    ++it;
    EXPECT_EQ(*it, 1);
    ++it;
    EXPECT_TRUE(it == lst.end());
}

TEST(PmrForwardList, NoLeaksWhenListDestroyed)
{
    std::stringstream capture;
    {
        CerrRedirect redirect(capture);

        FixedPoolResource pool(100 * 1024);
        {
            Forward_List<std::string> lst(&pool);

            for (int i = 0; i < 1000; ++i)
                lst.emplace_front("eqwrreywtryhytjtjujyujikyfhgfghf " + std::to_string(i));
        }

        std::string output = capture.str();
        EXPECT_TRUE(output.empty()) << "LEAKS: " << output;
    }
}

TEST(PmrForwardList, ClearReusesMemoryFromPool)
{
    FixedPoolResource pool(1024);

    void *first_node_ptr = nullptr;

    {
        Forward_List<int> lst(&pool);
        lst.emplace_front(111);
        first_node_ptr = &lst.front();
    }

    {
        Forward_List<int> lst2(&pool);
        lst2.emplace_front(222);
        void *reused_ptr = &lst2.front();
        EXPECT_EQ(first_node_ptr, reused_ptr);
    }
}
