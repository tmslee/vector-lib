#include <vector_lib/vector.hpp>

#include <gtest/gtest.h>

TEST(VectorTest, DefaultConstructor) {
    vector_lib::Vector<int> v;
    EXPECT_TRUE(v.empty());
    EXPECT_EQ(v.size(), 0);
    EXPECT_EQ(v.capacity(), 0);
}