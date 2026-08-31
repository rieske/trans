#ifndef TEST_COMPILE_TO_IR_H_
#define TEST_COMPILE_TO_IR_H_

#include <string>

// Test-only. Same GNU + builtin-typedef installs as Compiler::compile.
// Source is scanned as-is (no gcc -E).
std::string compileToIr(const std::string& source);

#endif
