#include "memory_resource.h"
#include <gtest/gtest.h>

TEST(FixedPoolResource, AllocateAndDeallocate)
{
    FixedPoolResource pool(1024);

    void *p1 = pool.allocate(64, alignof(std::max_align_t));
    ASSERT_NE(p1, nullptr);

    void *p2 = pool.allocate(128, alignof(std::max_align_t));
    ASSERT_NE(p2, nullptr);

    pool.deallocate(p1, 64, alignof(std::max_align_t));
    pool.deallocate(p2, 128, alignof(std::max_align_t));
}

TEST(FixedPoolResource, ReuseMemory)
{
    FixedPoolResource pool(1024);

    void *p1 = pool.allocate(32, 8);
    pool.deallocate(p1, 32, 8);

    void *p2 = pool.allocate(32, 8);
    EXPECT_EQ(p1, p2);

    pool.deallocate(p2, 32, 8);
}

TEST(FixedPoolResource, AllocateMultipleBlocks)
{
    FixedPoolResource pool(1024);
    std::vector<void *> blocks;

    for (int i = 0; i < 10; i++)
    {
        void *p = pool.allocate(16, 8);
        blocks.push_back(p);
    }

    for (auto p : blocks)
        pool.deallocate(p, 16, 8);
}
