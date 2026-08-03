#ifndef LEXICALSESSION_H_
#define LEXICALSESSION_H_

// Per-translation-unit lexical state shared by scanner, TokenStream, and AST build.
// Owned by the compile pipeline (Compiler::compileTranslationUnit); never process-static.

#include "EnumConstantRegistry.h"
#include "TypedefRegistry.h"

namespace scanner {

// Stack-owned only: FA holds a raw pointer into typedefs, so copies/moves would
// dangle. Share by reference (Compiler owns the session for one TU).
struct LexicalSession {
    TypedefRegistry typedefs;
    EnumConstantRegistry enums;

    LexicalSession() = default;
    LexicalSession(const LexicalSession&) = delete;
    LexicalSession& operator=(const LexicalSession&) = delete;
    LexicalSession(LexicalSession&&) = delete;
    LexicalSession& operator=(LexicalSession&&) = delete;
};

} // namespace scanner

#endif // LEXICALSESSION_H_
