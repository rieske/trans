#include "TokenFilter.h"

#include "util/StringLiteralDecode.h"

#include <utility>
#include <vector>

namespace scanner {
namespace {

bool isDroppedMarker(const std::string& lexeme) {
    return lexeme == "__extension__";
}

bool isAsmPrefix(const std::string& lexeme) {
    return lexeme == "volatile"
            || lexeme == "const"
            || lexeme == "goto"
            || lexeme == "inline";
}

bool isAttribute(const std::string& lexeme) {
    return lexeme == "__attribute__";
}

bool isAsmKeyword(const std::string& lexeme) {
    return lexeme == "__asm__" || lexeme == "__asm" || lexeme == "asm";
}

bool isWideStringPrefixToken(const Token& t) {
    return (t.id == "id" || t.id == "typedef_name")
            && (t.lexeme == "L" || t.lexeme == "u" || t.lexeme == "U" || t.lexeme == "u8");
}

const char* canonicalKeyword(const std::string& lexeme) {
    static const std::pair<const char*, const char*> kMap[] = {
            { "__const", "const" },
            { "__const__", "const" },
            { "__signed", "signed" },
            { "__signed__", "signed" },
            { "__volatile", "volatile" },
            { "__volatile__", "volatile" },
            { "__inline", "inline" },
            { "__inline__", "inline" },
            { "__restrict", "restrict" },
            { "__restrict__", "restrict" },
            { "__typeof", "typeof" },
            { "__typeof__", "typeof" },
            { "__int128", "long" },
            { "__int128_t", "long" },
    };
    for (const auto& entry : kMap) {
        if (lexeme == entry.first) {
            return entry.second;
        }
    }
    return nullptr;
}

} // namespace

TokenFilter::TokenFilter(std::function<Token()> raw) :
        raw_ { std::move(raw) } {
}

void TokenFilter::pushFront(Token t) {
    pending_.emplace_front(std::move(t));
}

Token TokenFilter::nextRaw() {
    if (!pending_.empty()) {
        Token t { pending_.front() };
        pending_.pop_front();
        return t;
    }
    return raw_();
}

void TokenFilter::skipBalancedParenGroup() {
    int depth = 1;
    while (depth > 0) {
        Token t = nextRaw();
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

bool TokenFilter::isStringToken(const Token& t) {
    return t.id == "string";
}

Token TokenFilter::nextBaseFiltered() {
    for (;;) {
        Token t = nextRaw();
        if (t.id == Token::END) {
            return t;
        }

        if (const char* canon = canonicalKeyword(t.lexeme)) {
            return Token { canon, canon, t.context };
        }
        if (t.lexeme == "__uint128_t") {
            pushFront(Token { "long", "long", t.context });
            return Token { "unsigned", "unsigned", t.context };
        }

        if (isDroppedMarker(t.lexeme)) {
            continue;
        }

        if (isAttribute(t.lexeme)) {
            Token next = nextRaw();
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
            std::vector<Token> prefixes;
            for (;;) {
                Token next = nextBaseFiltered();
                if (next.id == Token::END) {
                    for (auto it = prefixes.rbegin(); it != prefixes.rend(); ++it) {
                        pushFront(*it);
                    }
                    return next;
                }
                if (isAsmPrefix(next.lexeme)) {
                    prefixes.push_back(next);
                    continue;
                }
                if (next.lexeme == "(") {
                    skipBalancedParenGroup();
                    break;
                }
                pushFront(next);
                for (auto it = prefixes.rbegin(); it != prefixes.rend(); ++it) {
                    pushFront(*it);
                }
                break;
            }
            continue;
        }

        return t;
    }
}

Token TokenFilter::nextToken() {
    Token t = nextBaseFiltered();
    if (t.id == Token::END) {
        return t;
    }

    if (isWideStringPrefixToken(t)) {
        Token next = nextBaseFiltered();
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

Token TokenFilter::finishStringToken(const Token& first) {
    auto takeInterior = [](const Token& t) {
        auto bytes = util::decodeStringLiteralBytes(t.lexeme);
        if (!bytes.empty() && bytes.back() == '\0') {
            bytes.pop_back();
        }
        return bytes;
    };

    std::vector<unsigned char> bytes;
    bool glued = false;
    auto appendString = [&](const Token& t) {
        auto piece = takeInterior(t);
        if (!glued) {
            bytes = takeInterior(first);
            glued = true;
        }
        bytes.insert(bytes.end(), piece.begin(), piece.end());
    };

    for (;;) {
        Token next = nextBaseFiltered();
        if (next.id == Token::END) {
            break;
        }
        if (isStringToken(next)) {
            appendString(next);
            continue;
        }
        if (isWideStringPrefixToken(next)) {
            Token after = nextBaseFiltered();
            if (isStringToken(after)) {
                appendString(after);
                continue;
            }
            pushFront(after);
            pushFront(next);
            break;
        }
        pushFront(next);
        break;
    }
    if (!glued) {
        return first;
    }
    return Token { "string", util::encodeStringLiteralToken(bytes), first.context };
}

} // namespace scanner
