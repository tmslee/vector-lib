# vector-lib dev log

## 2026-02-12 — Project Setup

### What was done
- Initialized Git repo, created GitHub remote (`tmslee/vector-lib`)
- Set up CMake build system (C++20, strict warnings, ASan/TSan/UBSan support)
- Created header-only library skeleton with `Vector<T>` class template
- Integrated Google Test via FetchContent
- Added `.clang-format`, `.clang-tidy`, `.gitignore`

### Key decisions
- **Header-only**: `Vector<T>` is a template, so everything lives in headers. Library target is `INTERFACE`.
- **Target-scoped warnings**: Using `target_compile_options(INTERFACE)` instead of global `add_compile_options` to avoid polluting third-party code (learned this when gtest failed with `-Wdouble-promotion`).
- **Allocator parameter from day one**: `template <typename T, typename Allocator = std::allocator<T>>` — not used yet, but the signature is correct for when we implement custom allocator support.

### Lessons learned
- `add_compile_options()` is global — it applies to *all* targets including third-party code fetched via FetchContent. Use `target_compile_options()` to scope warnings to your own targets only. This is also just better modern CMake practice.

### Project structure
```
vector-lib/
├── include/vector_lib/vector.hpp
├── tests/vector_test.cpp
├── tests/CMakeLists.txt
├── docs/devlog.md
├── .github/workflows/ci.yml
├── CMakeLists.txt
├── .gitignore
├── .clang-format
└── .clang-tidy
```

### CI pipeline
- **Build & test**: Debug + Release matrix
- **Sanitizers**: ASan, UBSan, TSan (TSan ready for when threading is added)
- **Format check**: `clang-format --dry-run --Werror` on all source files

---

## 2026-02-12 — push_back & Memory Management (feat/push-back)

### What was done
- Implemented proper destructor (destroy elements, then deallocate memory)
- Added `push_back(const T&)` and `push_back(T&&)` with automatic reallocation
- Added `clear()` and `operator[]` (const and non-const)
- Added `grow()` with 2x growth factor for amortized O(1) push_back

### Key concepts
- **Separation of allocation and construction**: `allocate()` gets raw memory, `construct()` builds objects in it, `destroy()` calls destructors without freeing, `deallocate()` frees the memory block. Necessary because capacity > size — uninitialized slots exist in the buffer.
- **`std::allocator_traits`**: never call the allocator directly. The traits layer provides defaults for custom allocators that don't define every method.
- **Two `push_back` overloads**: `const T&` copies lvalues, `T&&` moves rvalues.
- **`[[no_unique_address]]`**: C++20 attribute that optimizes away storage for stateless types like `std::allocator`.

### Lessons learned
- `-Wsign-conversion` catches implicit `int` → `size_type` conversions. Use `std::size_t` for loop indices over container sizes, and `static_cast<int>()` when you intentionally narrow.

---

## 2026-02-12 — Rule of 5: Copy & Move Semantics (feat/copy-move)

### What was done
- Copy constructor: deep copy with tight allocation (`capacity = size`, not `other.capacity`)
- Copy assignment: copy-and-swap idiom
- Move constructor: `noexcept` pointer steal, O(1)
- Move assignment: `noexcept`, cleanup then steal
- `swap()` member function

### Key concepts
- **Rule of 5**: if you define any of {destructor, copy ctor, copy assign, move ctor, move assign}, define all five. Our custom destructor means the compiler won't generate correct copy/move operations.
- **Copy-and-swap**: `operator=(const Vector& other)` creates a temporary copy, swaps with it, and the old data dies with `tmp`. Gives the **strong exception guarantee** for free — if the copy throws, `*this` is untouched.
- **`noexcept` on move operations**: critical contract. `std::vector` checks `is_nothrow_move_constructible` to decide whether to move or copy during reallocation. Without `noexcept`, containers fall back to copying for safety.
- **`select_on_container_copy_construction`**: lets the allocator decide how it should be copied — important for stateful allocators.
- **Tight allocation on copy**: copy allocates exactly `other.size_`, not `other.capacity_`. Capacity is an implementation detail of growth history, not data.

---

## 2026-02-12 — Exception Safety (feat/exception-safety)

### What was done
- `grow()` now uses `std::move_if_noexcept` — moves when T's move ctor is noexcept, copies otherwise
- try/catch rollback in `grow()`: on failure, destroy constructed elements in new buffer, deallocate, rethrow
- try/catch rollback in copy constructor: incremental `size_` tracking, clean up on throw
- Both provide the **strong exception guarantee**
- Added `ThrowOnCopy` test type to verify exception safety

### Key concepts
- **Three exception safety levels**: nothrow (can't throw), strong (if it throws, state unchanged), basic (valid but unspecified state). `grow()` and copy ctor now provide strong.
- **`std::move_if_noexcept`**: returns `T&&` if `is_nothrow_move_constructible`, `const T&` otherwise. This is how `std::vector` decides whether to move or copy during reallocation — moving is faster but if it throws mid-way, the original data is destroyed. Copying preserves originals as backup.
- **Rollback pattern**: track how many elements were constructed (`constructed` counter), catch all exceptions, destroy what was built, deallocate, rethrow. The original buffer is never touched until all new elements are safely constructed.
- **Copy-and-swap inherits strong guarantee**: copy assignment uses copy ctor (now exception-safe) + swap (noexcept). The strong guarantee composes naturally.

---

## 2026-02-12 — emplace_back & Perfect Forwarding (feat/emplace)

### What was done
- Added `emplace_back(Args&&... args)` — constructs elements in-place, forwarding arguments directly to T's constructor
- Updated `push_back` to return `reference` (matching C++17 `std::vector` behavior)
- Added `#include <utility>` for `std::forward` (previously relying on transitive include from `<memory>`)

### Key concepts
- **Variadic templates**: `template <typename... Args>` accepts a *parameter pack* — zero or more type parameters. The `...` syntax is the *pack expansion* operator. This lets `emplace_back` accept any number of constructor arguments without needing separate overloads.
- **Forwarding references**: `Args&&` where `Args` is a deduced template parameter is a *forwarding reference* (not an rvalue reference). It binds to both lvalues and rvalues, preserving the original value category.
- **Perfect forwarding**: `std::forward<Args>(args)...` preserves each argument's value category when passing it to the constructor. Without it, all arguments decay to lvalues inside the function body, defeating move semantics.
- **In-place construction**: `push_back(Point(1, 2))` creates a temporary then moves it. `emplace_back(1, 2)` constructs directly in the buffer — no temporary, no move. The arguments are forwarded straight to `allocator_traits::construct`.

### Lessons learned
- `-Wshadow` catches constructor parameters that shadow member variables. `Point(int x, int y)` shadows `Point::x` and `Point::y`. Fix: use different parameter names (`px`, `py`).
- Always explicitly include headers you use (`<utility>` for `std::forward`/`std::move`). Transitive includes work until they don't — a different compiler or standard library version may not pull them in.

### Next up
- Iterators (`begin`/`end`, `cbegin`/`cend`) and range-based for loop support
