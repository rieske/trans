#include "TokenStream.h"

#include "scanner/TypedefRegistry.h"

#include <optional>
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

TokenStream::TokenStream(std::function<scanner::Token()> scan, scanner::TypedefRegistry& typedefs) :
    scan { std::move(scan) },
    typedefs_ { typedefs },
    currentToken { this->scan() }
{
}

// Transitions use the reclassified token id so shadows and type promotions
// feed roleAfter (not the raw FA id).
void TokenStream::advanceIdContext(const scanner::Token& token) {
    if (auto next = roleAfter(token.id)) {
        idContext_ = *next;
    }
}

scanner::Token TokenStream::reclassify(const scanner::Token& token) const {
    if (token.id != "id" && token.id != "typedef_name") {
        return token;
    }
    if (typedefs_.isIdentifierShadow(token.lexeme)) {
        return scanner::Token { "id", token.lexeme, token.context };
    }
    if (!typedefs_.has(token.lexeme)) {
        if (token.id == "typedef_name") {
            return scanner::Token { "id", token.lexeme, token.context };
        }
        return token;
    }
    if (idContext_ == LexIdContext::AsIdentifier) {
        return scanner::Token { "id", token.lexeme, token.context };
    }
    return scanner::Token { "typedef_name", token.lexeme, token.context };
}

scanner::Token TokenStream::getCurrentToken() const {
    const scanner::Token& raw = forgedToken ? *forgedToken : *currentToken;
    return reclassify(raw);
}

scanner::Token TokenStream::nextToken() {
    scanner::Token consumed = getCurrentToken();
    advanceIdContext(consumed);
    // Brace scopes bound typedef-name object shadows (see TypedefRegistry).
    // Parameter shadows flush on the next `{` (not body-only; intermediate braces
    // mid-param-list can flush early - product limit). Prototypes clear on `;`.
    if (consumed.id == "{") {
        typedefs_.pushIdentifierShadowScope();
        typedefs_.flushPendingParameterShadows();
    } else if (consumed.id == "}") {
        typedefs_.popIdentifierShadowScope();
    } else if (consumed.id == ";") {
        // Prototypes: drop pending param shadows so later typedef uses stay type.
        typedefs_.clearPendingParameterShadows();
    }
    if (forgedToken) {
        forgedToken.reset();
    } else {
        currentToken.emplace(scan());
    }
    return getCurrentToken();
}

void TokenStream::forgeToken(scanner::Token forgedToken) {
    this->forgedToken.emplace(forgedToken);
}

bool TokenStream::currentTokenIsForged() const {
    return static_cast<bool>(forgedToken);
}

} // namespace parser
