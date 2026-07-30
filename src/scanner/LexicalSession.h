#ifndef LEXICALSESSION_H_
#define LEXICALSESSION_H_

// Per-translation-unit lexical state. Owned by the compile pipeline
// (Compiler::compile); shared with the scanner now, and with TokenStream/AST
// in later Phase-2 slices. Never process-static.

#include "EnumConstantRegistry.h"
#include "TypedefRegistry.h"

namespace scanner {

// Stack-owned only: FA holds a raw pointer into typedefs, so copies/moves would
// dangle. Share by reference (Compiler::compile owns the session for one TU).
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
