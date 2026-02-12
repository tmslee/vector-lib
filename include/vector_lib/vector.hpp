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

   private:
    using alloc_traits = std::allocator_traits<Allocator>;

   public:
    // --- ctor/dtor ---
    Vector() noexcept = default;

    ~Vector() {
        clear();
        if (data_) {
            alloc_traits::deallocate(alloc_, data_, capacity_);
        }
    }

    // --- capacity ---
    [[nodiscard]] bool empty() const noexcept {
        return size_ == 0;
    }
    [[nodiscard]] size_type size() const noexcept {
        return size_;
    }
    [[nodiscard]] size_type capacity() const noexcept {
        return capacity_;
    }

    // --- modifiers ---
    void push_back(const T& value) {
        if (size_ == capacity_) {
            grow();
        }
        alloc_traits::construct(alloc_, data_ + size_, value);
        ++size_;
    }

    void push_back(T&& value) {
        if (size_ == capacity_) {
            grow();
        }
        alloc_traits::construct(alloc_, data_ + size_, std::move(value));
        ++size_;
    }

    void clear() noexcept {
        for (size_type i = 0; i < size_; ++i) {
            alloc_traits::destroy(alloc_, data_ + i);
        }
        size_ = 0;
    }

    // ---element access ---
    reference operator[](size_type index) {
        return data_[index];
    }
    const_reference operator[](size_type index) const {
        return data_[index];
    }

   private:
    void grow() {
        size_type new_cap = capacity_ == 0 ? 1 : capacity_ * 2;
        pointer new_data = alloc_traits::allocate(alloc_, new_cap);

        for (size_type i = 0; i < size_; ++i) {
            alloc_traits::construct(alloc_, new_data + i, std::move(data_[i]));
            alloc_traits::destroy(alloc_, data_ + i);
        }

        if (data_) {
            alloc_traits::deallocate(alloc_, data_, capacity_);
        }

        data_ = new_data;
        capacity_ = new_cap;
    }

    pointer data_ = nullptr;
    size_type size_ = 0;
    size_type capacity_ = 0;
    [[no_unique_address]] Allocator alloc_{};
};

}  // namespace vector_lib

#endif