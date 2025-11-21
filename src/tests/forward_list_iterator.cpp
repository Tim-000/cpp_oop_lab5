#include "../include/forward_list.h"
#include <gtest/gtest.h>

TEST(ForwardListIterator, BasicIteration)
{
    Forward_List<int> lst;
    lst.push_front(3);
    lst.push_front(2);
    lst.push_front(1);

    auto it = lst.begin();
    EXPECT_EQ(*it, 1);
    ++it;
    EXPECT_EQ(*it, 2);
    ++it;
    EXPECT_EQ(*it, 3);
    ++it;
    EXPECT_TRUE(it == lst.end());
}
