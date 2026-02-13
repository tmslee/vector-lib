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
- **Separation of allocation and construction**: `allocate()` gets raw memory, `construct()` builds objects in it, `destroy()` calls destructors without freeing, `deallocate()` frees the memory block. This is necessary because capacity > size — uninitialized slots exist in the buffer.
- **`std::allocator_traits`**: never call the allocator directly. The traits layer provides defaults for custom allocators that don't define every method.
- **Two `push_back` overloads**: `const T&` copies lvalues, `T&&` moves rvalues. This is the core of move semantics — `v.push_back(std::move(x))` avoids a copy.
- **`[[no_unique_address]]`**: C++20 attribute that lets the compiler optimize away storage for stateless types (like `std::allocator`). Without it, the allocator member takes 1 byte + padding.

### Lessons learned
- `-Wsign-conversion` catches implicit `int` → `size_type` conversions. Use `std::size_t` for loop indices over container sizes, and `static_cast<int>()` when you intentionally narrow.
- ASan validates destructor correctness — if you leak, it tells you exactly where the allocation happened.

### Next up
- Rule of 5 (copy/move constructors and assignment operators)
- Exception safety during reallocation
