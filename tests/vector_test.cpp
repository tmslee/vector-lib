#include <gtest/gtest.h>

#include <string>
#include <vector_lib/vector.hpp>

TEST(VectorTest, DefaultConstructor) {
    vector_lib::Vector<int> v;
    EXPECT_TRUE(v.empty());
    EXPECT_EQ(v.size(), 0);
    EXPECT_EQ(v.capacity(), 0);
}

TEST(VectorTest, PushBackLvalue) {
    vector_lib::Vector<int> v;
    int x = 42;
    v.push_back(x);
    EXPECT_EQ(v.size(), 1);
    EXPECT_EQ(v[0], 42);
}

TEST(VectorTest, PushBackRvalue) {
    vector_lib::Vector<std::string> v;
    v.push_back("hello");
    EXPECT_EQ(v.size(), 1);
    EXPECT_EQ(v[0], "hello");
}

TEST(VectorTest, PushBackGrowth) {
    vector_lib::Vector<int> v;
    for (std::size_t i = 0; i < 100; ++i) {
        v.push_back(static_cast<int>(i));
    }
    EXPECT_EQ(v.size(), 100);
    for (std::size_t i = 0; i < 100; ++i) {
        EXPECT_EQ(v[i], static_cast<int>(i));
    }
}

TEST(VectorTest, Clear) {
    vector_lib::Vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    v.clear();
    EXPECT_TRUE(v.empty());
    EXPECT_EQ(v.size(), 0);
    EXPECT_GE(v.capacity(), 3);
}

TEST(VectorTest, DestructorFreesMemory) {
    auto* v = new vector_lib::Vector<std::string>();
    v->push_back("test");
    v->push_back("memory");
    delete v;
}
