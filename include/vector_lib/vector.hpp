#ifndef VECTOR_LIB_VECTOR_HPP
#define VECTOR_LIB_VECTOR_HPP

#include <cstddef>
#include <memory>

namespace vector_lib {

template <typename T, typename Allocator = std::allocator<T>>
class Vector {
public:
    // --- type aliases ---
    using value_type = T;
    using allocator_type = Allocator;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;

    // --- ctor/dtor ---
    Vector() noexcept = default;
    ~Vector() = default;

    // --- capacity ---
    [[nodiscard]] bool empty() const noexcept { return size_ == 0; }
    [[nodiscard]] size_type size() const noexcept { return size_; }
    [[nodiscard]] size_type capacity() const noexcept { return capacity_; }

private:
    pointer data_ = nullptr;
    size_type size_ = 0;
    size_type capacity_ = 0;
};

} //namespace vector_lib

#endif