#include "TokenFilter.h"

#include "scanner/LexicalSession.h"
#include "scanner/Scanner.h"
#include "util/StringLiteralDecode.h"

#include <string_view>
#include <utility>
#include <vector>

namespace scanner {
namespace {

bool isDroppedMarker(std::string_view lexeme) {
    return lexeme == "__extension__";
}

bool isAsmPrefix(std::string_view lexeme) {
    return lexeme == "volatile"
            || lexeme == "const"
            || lexeme == "goto"
            || lexeme == "inline";
}

bool isAttribute(std::string_view lexeme) {
    return lexeme == "__attribute__";
}

bool isPackedAttribute(std::string_view lexeme) {
    return lexeme == "packed" || lexeme == "__packed__";
}

bool isTransparentUnionAttribute(std::string_view lexeme) {
    return lexeme == "transparent_union" || lexeme == "__transparent_union__";
}

bool isAsmKeyword(std::string_view lexeme) {
    return lexeme == "__asm__" || lexeme == "__asm" || lexeme == "asm";
}

bool isWideStringPrefixToken(const Token& t) {
    return (t.cls == TokenClass::Id || t.cls == TokenClass::TypedefName)
            && (t.lexeme == "L" || t.lexeme == "u" || t.lexeme == "U" || t.lexeme == "u8");
}

const char* isoKeywordAlias(std::string_view lexeme) {
    if (lexeme == "_Noreturn") {
        return "noreturn";
    }
    if (lexeme == "_Bool") {
        return "bool";
    }
    return nullptr;
}

const char* gnuKeywordAlias(std::string_view lexeme) {
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
    };
    for (const auto& entry : kMap) {
        if (lexeme == entry.first) {
            return entry.second;
        }
    }
    return nullptr;
}

} // namespace

TokenFilter::TokenFilter(std::function<Token()> raw, bool gnuExtensions, LexicalSession* session) :
        raw_ { std::move(raw) },
        gnuExtensions_ { gnuExtensions },
        session_ { session } {
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
    skipParenGroup(false);
}

void TokenFilter::skipAttributeGroup() {
    skipParenGroup(true);
}

void TokenFilter::skipParenGroup(bool noteTypeAttributes) {
    int depth = 1;
    while (depth > 0) {
        Token t = nextRaw();
        if (t.cls == TokenClass::End) {
            break;
        }
        if (noteTypeAttributes && session_) {
            if (isPackedAttribute(t.lexeme)) {
                session_->recordPacked.notePacked();
            }
            if (isTransparentUnionAttribute(t.lexeme)) {
                session_->transparentUnion.note();
            }
        }
        if (t.lexeme == "(") {
            ++depth;
        } else if (t.lexeme == ")") {
            --depth;
        }
    }
}

bool TokenFilter::isStringToken(const Token& t) {
    return t.cls == TokenClass::String;
}

Token TokenFilter::nextBaseFiltered() {
    for (;;) {
        Token t = nextRaw();
        if (t.cls == TokenClass::End) {
            return t;
        }

        if (const char* canon = isoKeywordAlias(t.lexeme)) {
            return Token { canon, canon, t.context };
        }
        if (gnuExtensions_) {
            if (const char* canon = gnuKeywordAlias(t.lexeme)) {
                return Token { canon, canon, t.context };
            }
        }

        if (gnuExtensions_ && isDroppedMarker(t.lexeme)) {
            continue;
        }

        if (gnuExtensions_ && isAttribute(t.lexeme)) {
            Token next = nextRaw();
            if (next.cls == TokenClass::End) {
                return next;
            }
            if (next.lexeme == "(") {
                skipAttributeGroup();
            } else {
                pushFront(next);
            }
            continue;
        }

        if (gnuExtensions_ && isAsmKeyword(t.lexeme)) {
            std::vector<Token> prefixes;
            for (;;) {
                Token next = nextBaseFiltered();
                if (next.cls == TokenClass::End) {
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

        if (session_ && (t.lexeme == "struct" || t.lexeme == "union")) {
            session_->recordPacked.noteStructOrUnionToken();
        }
        return t;
    }
}

Token TokenFilter::nextToken() {
    Token t = nextBaseFiltered();
    if (t.cls == TokenClass::End) {
        return t;
    }

    if (isWideStringPrefixToken(t)) {
        Token next = nextBaseFiltered();
        if (!isStringToken(next)) {
            pushFront(next);
            return t;
        }
        return finishStringToken(next, t.lexeme);
    }

    if (isStringToken(t)) {
        return finishStringToken(t);
    }

    return t;
}

Token TokenFilter::finishStringToken(const Token& first, std::string prefix) {
    std::vector<std::string> pieces;
    pieces.push_back(prefix.empty() ? first.lexeme : prefix + first.lexeme);

    for (;;) {
        Token next = nextBaseFiltered();
        if (next.cls == TokenClass::End) {
            break;
        }
        if (isStringToken(next)) {
            pieces.push_back(next.lexeme);
            continue;
        }
        if (isWideStringPrefixToken(next)) {
            Token after = nextBaseFiltered();
            if (isStringToken(after)) {
                pieces.push_back(next.lexeme + after.lexeme);
                continue;
            }
            pushFront(after);
            pushFront(next);
            break;
        }
        pushFront(next);
        break;
    }
    if (pieces.size() == 1) {
        return Token { "string", pieces.front(), first.context };
    }

    std::string wide;
    bool sawU8 = false;
    bool mixedWide = false;
    for (const std::string& piece : pieces) {
        const auto quote = piece.find('"');
        const std::string piecePrefix = quote == std::string::npos ? std::string {} : piece.substr(0, quote);
        if (piecePrefix == "u8") {
            sawU8 = true;
            continue;
        }
        if (util::stringLiteralUnitBytes(piece) == 1) {
            continue;
        }
        if (wide.empty()) {
            wide = piecePrefix;
        } else if (piecePrefix != wide) {
            mixedWide = true;
        }
    }
    if (!wide.empty()) {
        if (sawU8 || mixedWide) {
            throw LexError { first.context,
                    "concatenation of string literals with conflicting encoding prefixes" };
        }
        return Token { "string", util::concatWideLiterals(wide, pieces), first.context };
    }

    std::vector<unsigned char> bytes;
    for (const std::string& piece : pieces) {
        auto interior = util::decodeStringLiteralBytes(piece);
        if (!interior.empty() && interior.back() == '\0') {
            interior.pop_back();
        }
        bytes.insert(bytes.end(), interior.begin(), interior.end());
    }
    return Token { "string", util::encodeStringLiteralToken(bytes), first.context };
}

} // namespace scanner
