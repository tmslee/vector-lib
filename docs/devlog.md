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

### Project structure
```
vector-lib/
├── include/vector_lib/vector.hpp
├── tests/vector_test.cpp
├── tests/CMakeLists.txt
├── docs/devlog.md
├── CMakeLists.txt
├── .gitignore
├── .clang-format
└── .clang-tidy
```

### Next up
- Implement destructor, `push_back`, and dynamic reallocation
- Rule of 5 (copy/move constructors and assignment operators)
- Exception safety during reallocation
