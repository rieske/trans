trans
=====

[![Actions Status](https://github.com/rieske/trans/workflows/build/badge.svg)](https://github.com/rieske/trans/actions)
[![Build Status](https://travis-ci.org/rieske/trans.png?branch=master)](https://travis-ci.org/rieske/trans)
[![Coverage Status](https://coveralls.io/repos/github/rieske/trans/badge.svg?branch=master)](https://coveralls.io/github/rieske/trans?branch=master)

## About

A compiler for C-like programming language.

## Structure

### Scanner
A configurable finite automaton, recognizing lexemes in the character stream.
Configured using [scanner.lex](resources/configuration/scanner.lex) file.

### Parser/Parser Generator
A LALR parser generator and a parser that recognizes an augmented C grammar.
The grammar is extended with input/output operations - which I intend to fix at some point by linking the executables with the
standard C library instead. Also the typedef resoluton is not implemented yet, thus typedefs are disabled.
The parser reads the generated [parsing_table](resources/configuration/parsing_table) file, generated from
[grammar.bnf](resources/configuration/grammar.bnf).
A custom/changed grammar can be passed to the `trans` program using `./trans -g<path_to_grammar_file>` and it will generate
a new parsing table in `logs/parsing_table`.

### Abstract Syntax Tree
Contains a hierarchy of language constructs accepting visitors for semantic analysis, code generation and output.

### Semantic Analyzer
An AST visitor at the core that orchestrates the semantic analysis.

### Code Generator
An AST visitor that generates the intermmediate code and an assembly code generator that visits the intermmediate code nodes
to generate the assembly code. Currently it generates 64bit code for the NASM assembler.

## Building

Prerequisites:
- cmake - at least 3.10
- make
- g++
- nasm - the assembler required to run the compiled compiler and its functional tests

```shell
./init.sh # to generate the workspace using cmake
cd build
make -j8 # where 8 is the number of worker threads
make test # to run the tests
```

To run the tests with verbose output, use `make test "ARGS=-V"`.

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
- Rewrote the code generator to generate 64bit NASM assembly code and made it extendible to support the GNU syntax

