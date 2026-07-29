#include "Scanner.h"

#include "Token.h"
#include "util/StringLiteralDecode.h"

#include <cstdio>
#include <vector>

namespace scanner {
namespace {

bool isDroppedMarker(const std::string& lexeme) {
    return lexeme == "__extension__"
            || lexeme == "__restrict"
            || lexeme == "__restrict__"
            || lexeme == "restrict"
            || lexeme == "__inline__"
            || lexeme == "__inline"
            || lexeme == "inline";
}

// Tokens that may sit between asm/__asm__ and the balanced paren group:
// classic qualifiers plus C/GNU statement prefixes (asm goto, asm inline).
bool isAsmPrefix(const std::string& lexeme) {
    return lexeme == "volatile"
            || lexeme == "__volatile__"
            || lexeme == "__volatile"
            || lexeme == "const"
            || lexeme == "__const"
            || lexeme == "__const__"
            || lexeme == "goto"
            || lexeme == "inline"
            || lexeme == "__inline__"
            || lexeme == "__inline";
}

bool isAttribute(const std::string& lexeme) {
    return lexeme == "__attribute__";
}

bool isAsmKeyword(const std::string& lexeme) {
    return lexeme == "__asm__" || lexeme == "asm";
}

bool isAttributeOrAsm(const std::string& lexeme) {
    return isAttribute(lexeme) || isAsmKeyword(lexeme);
}

bool isWideStringPrefix(const std::string& lexeme) {
    return lexeme == "L" || lexeme == "u" || lexeme == "U" || lexeme == "u8";
}

bool isFuncNameIdent(const std::string& lexeme) {
    return lexeme == "__func__"
            || lexeme == "__FUNCTION__"
            || lexeme == "__PRETTY_FUNCTION__";
}

std::string encodeStringBody(const std::vector<unsigned char>& bytes) {
    std::string out;
    out.reserve(bytes.size() * 4);
    for (unsigned char b : bytes) {
        switch (b) {
        case '\n': out += "\\n"; break;
        case '\t': out += "\\t"; break;
        case '\r': out += "\\r"; break;
        case '\a': out += "\\a"; break;
        case '\b': out += "\\b"; break;
        case '\f': out += "\\f"; break;
        case '\v': out += "\\v"; break;
        case '\\': out += "\\\\"; break;
        case '"': out += "\\\""; break;
        default:
            if (b >= 0x20 && b <= 0x7e) {
                out += static_cast<char>(b);
            } else {
                char buf[5];
                std::snprintf(buf, sizeof(buf), "\\%03o", static_cast<unsigned>(b));
                out += buf;
            }
            break;
        }
    }
    return out;
}

} // namespace

Scanner::Scanner(std::string fileName, std::unique_ptr<FiniteAutomaton> stateMachine, LexicalSession& session) :
        translationUnit { fileName },
        automaton { std::move(stateMachine) },
        session_ { session } {
    automaton->setTypedefRegistry(&session_.typedefs);
}

void Scanner::pushFront(Token t) {
    pending.emplace_front(t);
}

Token Scanner::nextTokenUnfiltered() {
    if (!pending.empty()) {
        Token t { pending.front() };
        pending.pop_front();
        return t;
    }
    char currentCharacter;
    do {
        currentCharacter = translationUnit.getNextCharacter();
        automaton->updateState(currentCharacter);
    } while (!automaton->isAtFinalState() && currentCharacter != '\0');
    return { automaton->getAccumulatedToken(), automaton->getAccumulatedLexeme(),
            translationUnit.getContext() };
}

void Scanner::skipBalancedParenGroup() {
    // Post-E noise path: unclosed parens drain to END rather than erroring.
    int depth = 1;
    while (depth > 0) {
        Token t = nextTokenUnfiltered();
        if (t.id == Token::END) {
            break;
        }
        if (t.lexeme == "(") {
            ++depth;
        } else if (t.lexeme == ")") {
            --depth;
        }
    }
}

bool Scanner::isStringToken(const Token& t) {
    return t.id == "string";
}

Token Scanner::nextTokenBaseFiltered() {
    for (;;) {
        Token t = nextTokenUnfiltered();
        if (t.id == Token::END) {
            return t;
        }

        if (t.lexeme == "__const" || t.lexeme == "__const__") {
            return Token { "const", "const", t.context };
        }
        if (t.lexeme == "__signed__" || t.lexeme == "__signed") {
            return Token { "signed", "signed", t.context };
        }
        if (t.lexeme == "__volatile__" || t.lexeme == "__volatile") {
            return Token { "volatile", "volatile", t.context };
        }

        // C99 _Bool is 1 byte: expand to "unsigned" "char" (exact-id only so
        // XML_Bool is untouched).
        if (t.lexeme == "_Bool") {
            pushFront(Token { "char", "char", t.context });
            return Token { "unsigned", "unsigned", t.context };
        }

        // Extended integer / binary-float spellings from system headers after
        // gcc -E (linux/types.h __int128, glibc _FloatN). Approximate for parse
        // and layout only - not full 128-bit / _FloatN ABI.
        if (t.lexeme == "__int128") {
            pushFront(Token { "long", "long", t.context });
            return Token { "long", "long", t.context };
        }
        if (t.lexeme == "_Float32") {
            return Token { "float", "float", t.context };
        }
        if (t.lexeme == "_Float64" || t.lexeme == "_Float128"
                || t.lexeme == "_Float32x" || t.lexeme == "_Float64x") {
            return Token { "double", "double", t.context };
        }

        if (isFuncNameIdent(t.lexeme)) {
            return Token { "string", "\"\"", t.context };
        }

        if (isDroppedMarker(t.lexeme)) {
            continue;
        }

        if (isAttribute(t.lexeme)) {
            // Only skip a following balanced paren group; do not drop type
            // qualifiers/keywords that happen to look like asm prefixes.
            Token next = nextTokenUnfiltered();
            if (next.id == Token::END) {
                return next;
            }
            if (next.lexeme == "(") {
                skipBalancedParenGroup();
            } else {
                pushFront(next);
            }
            continue;
        }

        if (isAsmKeyword(t.lexeme)) {
            // Optional prefixes before the paren group (asm volatile (...),
            // asm goto (...), asm inline (...)).
            for (;;) {
                Token next = nextTokenUnfiltered();
                if (next.id == Token::END) {
                    return next;
                }
                if (isAsmPrefix(next.lexeme)) {
                    continue;
                }
                if (next.lexeme == "(") {
                    skipBalancedParenGroup();
                    break;
                }
                // No paren group: drop the asm keyword and any consumed prefixes
                // (prefixes were skipped with continue above, not restored).
                pushFront(next);
                break;
            }
            continue;
        }

        return t;
    }
}

Token Scanner::nextToken() {
    Token t = nextTokenBaseFiltered();
    if (t.id == Token::END) {
        return t;
    }

    // Wide / UTF prefixes: L"..." u"..." U"..." u8"..." -> plain string token.
    // Token members are const, so rebind via a fresh local rather than assign.
    // Lexical prefixes are independent of typedef tables (id or typedef_name).
    // Peek base-filtered so dropped markers between L and "..." are transparent
    // (unlike finishStringToken adjacency, which peeks unfiltered).
    if ((t.id == "id" || t.id == "typedef_name") && isWideStringPrefix(t.lexeme)) {
        Token next = nextTokenBaseFiltered();
        if (!isStringToken(next)) {
            pushFront(next);
            return t;
        }
        return finishStringToken(next);
    }

    if (isStringToken(t)) {
        return finishStringToken(t);
    }

    return t;
}

// C adjacent string concatenation (phase 6 after decode). Token is not
// assignable (const members), so accumulate bytes and build one result.
// decodeStringLiteralBytes includes trailing NUL; drop it when concatenating
// interiors so only the final token has one NUL when re-encoded as a source form.
//
// Adjacency peeks unfiltered tokens: dropped markers / attributes between
// literals are NOT transparent (GCC keeps non-adjacent strings separate).
// Push the separator back so nextTokenBaseFiltered can still drop it later.
Token Scanner::finishStringToken(const Token& first) {
    auto takeInterior = [](const Token& t) {
        auto bytes = util::decodeStringLiteralBytes(t.lexeme);
        if (!bytes.empty() && bytes.back() == '\0') {
            bytes.pop_back();
        }
        return bytes;
    };
    auto bytes = takeInterior(first);
    for (;;) {
        Token next = nextTokenUnfiltered();
        if (next.id == Token::END) {
            break;
        }
        // Noise between strings ends adjacency (do not glue across it).
        if (isDroppedMarker(next.lexeme) || isAttributeOrAsm(next.lexeme)) {
            pushFront(next);
            break;
        }
        if (isStringToken(next)) {
            auto right = takeInterior(next);
            bytes.insert(bytes.end(), right.begin(), right.end());
            continue;
        }
        if ((next.id == "id" || next.id == "typedef_name") && isWideStringPrefix(next.lexeme)) {
            Token after = nextTokenUnfiltered();
            if (isStringToken(after)) {
                auto right = takeInterior(after);
                bytes.insert(bytes.end(), right.begin(), right.end());
                continue;
            }
            pushFront(after);
            pushFront(next);
            break;
        }
        pushFront(next);
        break;
    }
    const std::string lexeme = "\"" + encodeStringBody(bytes) + "\"";
    return Token { "string", lexeme, first.context };
}

} // namespace scanner
