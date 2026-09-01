trans
=====

[![Actions Status](https://github.com/rieske/trans/actions/workflows/build.yml/badge.svg?branch=master)](https://github.com/rieske/trans/actions/workflows/build.yml?query=branch%3Amaster)
[![Coverage Status](https://coveralls.io/repos/github/rieske/trans/badge.svg?branch=master)](https://coveralls.io/github/rieske/trans?branch=master)

## About

A C compiler for x86-64 System V. Host `gcc` preprocesses and links. Default
assembly is AT&T/GAS; `-masm=intel` is Intel/NASM.

## Structure

### Scanner
A configurable finite automaton, recognizing lexemes in the character stream.
Configured using [scanner.lex](resources/configuration/scanner.lex) file.
`TokenFilter` then rewrites GNU spellings, strips attributes/asm, and concatenates
adjacent strings.

### Parser/Parser Generator
A LALR parser generator and a parser that recognizes C grammar.
The product table is compiled from [grammar.bnf](resources/configuration/grammar.bnf) at build time.

### Abstract Syntax Tree
Contains a hierarchy of language constructs accepting visitors for semantic analysis and code generation.

### Semantic Analyzer
An AST visitor at the core that orchestrates the semantic analysis.

### Code Generator
An AST visitor that generates intermediate code. `AssemblyGenerator` walks that IR
and `StackMachine` emits 64-bit assembly (AT&T/GAS by default, Intel/NASM with `-masm=intel`).

## Building

Prerequisites:
- cmake - at least 3.17
- a C++20 compiler (g++ or clang++)
- a build tool (Make or Ninja; the root `Makefile` shells out to CMake either way)
- gcc - preprocessor and linker
- GNU as - default assembler (AT&T functional tests)
- nasm - Intel assembler (`-masm=intel` functional tests)

From the repo root:

```shell
make              # configure on first run if needed, then build
make test         # build and run tests (ctest -j by default)
make test ARGS='-R parser -V'   # filter / verbose ctest
make test ARGS='-L functional'  # functional shards only (Intel and AT&T)
make test JOBS=1  # serial ctest
make coverage     # serial tests + build/coverage/lcov.info (needs lcov; Debug enables gcov by default)
make help         # list targets
```

Functional tests run as gtest **shards** (default 8, both dialects) so `make test` / `ctest -j` can parallelize them. New `TEST()` cases need no CMake changes. Parallelism for day-to-day runs is owned by the root Makefile (`JOBS`); `make coverage` always runs ctest serial so gcov `.gcda` files stay consistent. To change shard count, reconfigure:

```shell
cmake -S . -B build -DFUNCTIONAL_TEST_SHARDS=16
```

Equivalent CMake commands (no root Makefile):

```shell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DTRANS_ENABLE_COVERAGE=ON
cmake --build build -j$(nproc)
cd build && ctest --output-on-failure -j$(nproc)   # day-to-day
cd build && ctest --output-on-failure -j1            # before lcov / coverage
```

Or with presets (`CMakePresets.json`):

```shell
cmake --preset coverage    # Debug + gcov (same knobs as make configure)
cmake --build --preset coverage
ctest --preset coverage
# also: default (Release), debug, ci (coverage + serial tests)
```

`TRANS_ENABLE_COVERAGE` is an explicit option (not tied to the Debug config name). The root `Makefile` turns it on automatically when `BUILD_TYPE=Debug` (override with `make COVERAGE=OFF configure`).

## History
I started this project in my third year at the University as an assignment for Translation Methods course in Autumn of 2008.
Original duration of the assignment was three months and it was left buried in my backup drive since the end of the course.
By that time it contained a scanner, a LR1 parser generator and a generated LR1 parser, a semantic analyzer built into the AST,
and a code generator that would generate 32bit assembly code for the NASM assembler.


In Spring of 2014 I was browsing through my old backup drive and found this piece of student code I wrote.
Thought it would be nice to modernize it a bit and revive my skills in C++ and assembly and give my brain some interesting
puzzles to solve, different from those that I encounter in my daily job. So I pushed the whole code base "as is"
to this GitHub repo and started refactoring it, applying the coding practices of today as I know them.
The code was utterly ugly initially - lots of duplicated code, bad encapsulation, poor separation of concerns,
classes way too big, C and C++ library calls mixed together, no tests and so on and on...


As I started rewriting the translator top-down, I had trouble recalling the C++ language itself since I was programming
mostly in Java in the past five years and for this reason I'm sure that Java's influence on the coding style can be
observed a lot thoughout the new code - especially in the components that I tackled first - the scanner and the parser.
Anyway, I am trying to overcome the writing of Java in C++ and will do another round of refactoring on these when the time
permits.


Major functional improvements so far:
- Converted the LR1 parser generator to a LALR parser generator to save time and space
- Simplified inner scope resolution by prefixing inner scope local variables in the symbol table instead of manipulating the stack in the code generator
- Rewrote the code generator for 64-bit Intel/NASM and AT&T/GAS, with SysV AMD64 codegen locked against gcc


