#include "TokenStream.h"

#include "parser/Grammar.h"
#include "scanner/LexicalSession.h"

#include <optional>
#include <stdexcept>
#include <string_view>

namespace parser {

namespace {

// Closed transition membership: after consuming this token id, next name is
// AsType / AsIdentifier, or keep current role (nullopt). Extend via these
// helpers only - do not add one-off previous-token specials in nextToken.

bool isTagKeyword(std::string_view id) {
    return id == "struct" || id == "union" || id == "enum";
}

bool isMemberOp(std::string_view id) {
    return id == "." || id == "->";
}

// Declarator / unary '*': next name is an identifier.
bool isStar(std::string_view id) {
    return id == "*";
}

// After a typedef_name token, the next name is the object/declarator identifier.
bool isTypedefNameToken(std::string_view id) {
    return id == "typedef_name";
}

// Keywords, unaries (except '*'), assigns/relops, primaries that put the next
// name in expression position. Extend this table only - keep roleAfter closed.
constexpr std::string_view kExpressionCues[] = {
        "return", "else", "goto", "sizeof", "case",
        "++", "--", "+", "-", "&", "!", "~",
        "=", "+=", "-=", "*=", "/=", "%=",
        "<<=", ">>=", "&=", "^=", "|=",
        "==", "!=", "<", ">", "<=", ">=",
        "<<", ">>", "/", "%", "^", "|",
        "&&", "||", "?", "[",
        "id", "int_const", "char_const", "float_const", "string", "enumeration_const",
};

bool isExpressionCue(std::string_view id) {
    for (std::string_view cue : kExpressionCues) {
        if (cue == id) {
            return true;
        }
    }
    return false;
}

// Restart type position. Intentionally omits ':' (label/case vs ternary).
// ',' prefers type-list / multi-declarator type reuse (`void f(int a, size_t b)`)
// over multi-declarator object lists that reuse a typedef spelling as a name
// without a shadow (`int a, T` in an inner scope). Shadows cover the common
// object case; a full type-vs-expression lattice is out of product scope.
bool isTypeRestart(std::string_view id) {
    return id == ";" || id == "{" || id == "}" || id == ")" || id == "]"
            || id == "(" || id == ",";
}

// Primitive type-specifiers put the next name in declarator position (`int T`).
// Qualifiers (const/volatile) intentionally keep context (nullopt) so
// `const foo_t x` still treats foo_t as a typedef_name.
bool isPrimitiveTypeSpec(std::string_view id) {
    return id == "int" || id == "char" || id == "void" || id == "short"
            || id == "long" || id == "signed" || id == "unsigned"
            || id == "float" || id == "double";
}

std::optional<LexIdContext> roleAfter(std::string_view id) {
    if (isTagKeyword(id) || isMemberOp(id) || isStar(id) || isTypedefNameToken(id)
            || isExpressionCue(id) || isPrimitiveTypeSpec(id)) {
        return LexIdContext::AsIdentifier;
    }
    if (isTypeRestart(id)) {
        return LexIdContext::AsType;
    }
    return std::nullopt;
}

} // namespace

TokenStream::TokenStream(std::function<scanner::Token()> scan, scanner::LexicalSession& session,
        const Grammar& grammar) :
    scan { std::move(scan) },
    session_ { session },
    grammar_ { grammar },
    current_ { classifyAndStamp(this->scan()) },
    classifiedRevision_ { session.typedefs.revision() }
{
}

// Transitions use the reclassified token id so shadows and type promotions
// feed roleAfter (not the raw FA id).
void TokenStream::advanceIdContext(const scanner::Token& token) {
    if (auto next = roleAfter(token.id)) {
        idContext_ = *next;
    }
}

void TokenStream::setIdContext(LexIdContext context) {
    idContext_ = context;
    refreshCurrent();
}

scanner::Token TokenStream::classifyAndStamp(const scanner::Token& token) const {
    std::string id = token.id;
    if (id == "id" || id == "typedef_name") {
        if (session_.typedefs.isIdentifierShadow(token.lexeme)) {
            id = "id";
        } else if (!session_.isTypedef(token.lexeme)) {
            id = "id";
        } else if (idContext_ == LexIdContext::AsIdentifier) {
            id = "id";
        } else {
            id = "typedef_name";
        }
    }
    const auto symbolId = grammar_.trySymbolId(id);
    if (!symbolId) {
        throw std::logic_error { "TokenStream: not a grammar terminal: " + id };
    }
    return { std::move(id), token.lexeme, token.context, *symbolId };
}

void TokenStream::refreshCurrent() const {
    current_ = classifyAndStamp(current_);
    classifiedRevision_ = session_.typedefs.revision();
}

void TokenStream::installNext() {
    if (lookahead_) {
        current_ = classifyAndStamp(*lookahead_);
        lookahead_.reset();
    } else {
        current_ = classifyAndStamp(scan());
    }
    classifiedRevision_ = session_.typedefs.revision();
}

const scanner::Token& TokenStream::getCurrentToken() const {
    if (classifiedRevision_ != session_.typedefs.revision()) {
        refreshCurrent();
    }
    return current_;
}

const scanner::Token& TokenStream::peek() {
    if (!lookahead_) {
        lookahead_.emplace(scan());
    }
    return *lookahead_;
}

scanner::Token TokenStream::takeRaw() {
    scanner::Token taken = getCurrentToken();
    installNext();
    return taken;
}

const scanner::Token& TokenStream::nextToken() {
    const scanner::Token consumed = getCurrentToken();
    advanceIdContext(consumed);
    if (consumed.id == "{") {
        session_.enterBlock();
    } else if (consumed.id == "}") {
        session_.leaveBlock();
    } else if (consumed.id == ";") {
        session_.endDeclarators();
    }
    installNext();
    return getCurrentToken();
}

} // namespace parser
