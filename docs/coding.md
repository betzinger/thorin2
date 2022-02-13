# Coding & Debugging

[TOC]

This document comprises some information that is related to coding but does not directly deals with the API.

## Coding Style

Use the following coding convetions:
* class/type names in `CamelCase`
* constants as defined in an `enum` or via `static const` in `Camel_Snake_Case`
* macro names in `SNAKE_IN_ALL_CAPS`
* eveything else like variables, functions, etc. in `snake_case`
* use a traling underscore suffix for a `private_member_variable_`
* don't do that for a `public_member_variable`
* use `struct` for [plain old data](https://en.cppreference.com/w/cpp/named_req/PODType)
* use `class` for everything else
* visibility groups in this order:
    1. `public`
    2. `protected`
    3. `private`
* prefer `// C++-style comments` over `/* C-style comments */`
* use `/// three slashes for Doxygen` and [group](https://www.doxygen.nl/manual/grouping.html) your methods into logical units if possible

For all the other minute details like indentation width etc. use [clang-format](https://clang.llvm.org/docs/ClangFormat.html) and the provided `.clang-format` file in the root of the repository.
We found a full automatic run of `clang-format` a bit too intrusive.
In a compiler, you often have to do similar things where it makes sense to align the corresponding code logically instead of blindly obeying some formatting rules.
Simply, checkout plugins like the [Vim integration](https://clang.llvm.org/docs/ClangFormat.html#vim-integration) in order to only format the desired code region.

# Debugging

## Logging

## Breakpoints