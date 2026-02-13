#include <gtest/gtest.h>

#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector_lib/vector.hpp>

struct ThrowOnCopy {
    int value;
    explicit ThrowOnCopy(int v) : value(v) {}

    ThrowOnCopy(const ThrowOnCopy& other) : value(other.value) {
        if (value == 3) {
            throw std::runtime_error("copy failed");
        }
    }

    ThrowOnCopy(ThrowOnCopy&&) noexcept = default;
    ThrowOnCopy& operator=(const ThrowOnCopy&) = default;
    ThrowOnCopy& operator=(ThrowOnCopy&&) noexcept = default;
    ~ThrowOnCopy() = default;
};

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

TEST(VectorTest, CopyConstructor) {
    vector_lib::Vector<std::string> v;
    v.push_back("one");
    v.push_back("two");
    v.push_back("three");

    vector_lib::Vector<std::string> copy(v);
    EXPECT_EQ(copy.size(), 3);
    EXPECT_EQ(copy[0], "one");
    EXPECT_EQ(copy[1], "two");
    EXPECT_EQ(copy[2], "three");

    // verify deep copy — modifying original doesn't affect copy
    v.push_back("four");
    EXPECT_EQ(copy.size(), 3);
}

TEST(VectorTest, CopyAssignment) {
    vector_lib::Vector<int> a;
    a.push_back(1);
    a.push_back(2);

    vector_lib::Vector<int> b;
    b.push_back(10);

    b = a;
    EXPECT_EQ(b.size(), 2);
    EXPECT_EQ(b[0], 1);
    EXPECT_EQ(b[1], 2);

    // verify independence
    a.push_back(3);
    EXPECT_EQ(b.size(), 2);
}

TEST(VectorTest, SelfAssignment) {
    vector_lib::Vector<int> v;
    v.push_back(42);
    v = v;
    EXPECT_EQ(v.size(), 1);
    EXPECT_EQ(v[0], 42);
}

TEST(VectorTest, MoveConstructor) {
    vector_lib::Vector<std::string> v;
    v.push_back("hello");
    v.push_back("world");

    vector_lib::Vector<std::string> moved(std::move(v));
    EXPECT_EQ(moved.size(), 2);
    EXPECT_EQ(moved[0], "hello");
    EXPECT_EQ(moved[1], "world");

    // source should be empty
    EXPECT_EQ(v.size(), 0);
    EXPECT_EQ(v.capacity(), 0);
}

TEST(VectorTest, MoveAssignment) {
    vector_lib::Vector<int> a;
    a.push_back(1);
    a.push_back(2);

    vector_lib::Vector<int> b;
    b.push_back(99);

    b = std::move(a);
    EXPECT_EQ(b.size(), 2);
    EXPECT_EQ(b[0], 1);
    EXPECT_EQ(b[1], 2);

    EXPECT_EQ(a.size(), 0);
    EXPECT_EQ(a.capacity(), 0);
}

TEST(VectorTest, MoveConstructorIsNoexcept) {
    EXPECT_TRUE(std::is_nothrow_move_constructible_v<vector_lib::Vector<int>>);
    EXPECT_TRUE(std::is_nothrow_move_assignable_v<vector_lib::Vector<int>>);
}

TEST(VectorTest, GrowExceptionSafety) {
    // ThrowOnCopy has noexcept move, so grow() will move - no throw
    vector_lib::Vector<ThrowOnCopy> v;
    for (int i = 0; i < 10; ++i) {
        v.push_back(ThrowOnCopy(i));
    }
    EXPECT_EQ(v.size(), 10);
}

TEST(VectorTest, CopyConstructorExceptionSafety) {
    // element with value=3 throws on copy
    vector_lib::Vector<ThrowOnCopy> v;
    v.push_back(ThrowOnCopy(1));
    v.push_back(ThrowOnCopy(2));
    v.push_back(ThrowOnCopy(3));  // this one will throw when copied

    EXPECT_THROW({ vector_lib::Vector<ThrowOnCopy> copy(v); }, std::runtime_error);

    // original must be untouched
    EXPECT_EQ(v.size(), 3);
    EXPECT_EQ(v[0].value, 1);
    EXPECT_EQ(v[1].value, 2);
    EXPECT_EQ(v[2].value, 3);
}