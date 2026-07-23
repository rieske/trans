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
            || lexeme == "__inline__"
            || lexeme == "__inline"
            || lexeme == "inline";
}

bool isAttributeOrAsm(const std::string& lexeme) {
    return lexeme == "__attribute__"
            || lexeme == "__asm__"
            || lexeme == "asm";
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

std::string stringBody(const Token& t) {
    if (t.lexeme.size() >= 2 && t.lexeme.front() == '"' && t.lexeme.back() == '"') {
        return t.lexeme.substr(1, t.lexeme.size() - 2);
    }
    return t.lexeme;
}

} // namespace

Scanner::Scanner(std::string fileName, std::unique_ptr<FiniteAutomaton> stateMachine) :
        translationUnit { fileName },
        automaton { std::move(stateMachine) } {
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

        if (t.lexeme == "__const") {
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

        if (isAttributeOrAsm(t.lexeme)) {
            Token next = nextTokenUnfiltered();
            if (next.id == Token::END) {
                return next;
            }
            if (next.lexeme == "(") {
                skipBalancedParenGroup();
                continue;
            }
            pushFront(next);
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
    if (t.id == "id" && isWideStringPrefix(t.lexeme)) {
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
Token Scanner::finishStringToken(const Token& first) {
    auto bytes = util::decodeCEscapes(stringBody(first));
    for (;;) {
        Token next = nextTokenBaseFiltered();
        if (isStringToken(next)) {
            auto right = util::decodeCEscapes(stringBody(next));
            bytes.insert(bytes.end(), right.begin(), right.end());
            continue;
        }
        if (next.id == "id" && isWideStringPrefix(next.lexeme)) {
            Token after = nextTokenBaseFiltered();
            if (isStringToken(after)) {
                auto right = util::decodeCEscapes(stringBody(after));
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
    return Token { "string", "\"" + encodeStringBody(bytes) + "\"", first.context };
}

} // namespace scanner
