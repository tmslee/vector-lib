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

### Next up
- Implement destructor, `push_back`, and dynamic reallocation
- Rule of 5 (copy/move constructors and assignment operators)
- Exception safety during reallocation
