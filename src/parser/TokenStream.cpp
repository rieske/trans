#include "TokenStream.h"

#include "scanner/TypedefRegistry.h"

namespace parser {

namespace {

// C allows a declarator (or member name) to reuse a typedef name (e.g. git grep.h:
//   typedef int pcre2_match_data;
//   pcre2_match_data *pcre2_match_data;
// ). After '*' or a typedef_name type, the next typedef lexeme is the declarator id.
// After '.' / '->' a member name that collides with a typedef must also be an id.
// After struct/union/enum the following name is a tag in a separate namespace, so a
// colliding typedef (e.g. `struct FILE;`) must still lex as an identifier.
//
// Expression contexts such as `return` can only be followed by an expression, never a
// type, so a typedef-spelling name there must be an ordinary identifier (possibly a
// variable that shadows the typedef in the current scope).
bool preferIdentifierOverTypedef(const std::string& previousId) {
    return previousId == "*" || previousId == "typedef_name"
            || previousId == "." || previousId == "->"
            || previousId == "struct" || previousId == "union" || previousId == "enum"
            || previousId == "return" || previousId == "else" || previousId == "goto"
            || previousId == "sizeof" || previousId == "case"
            || previousId == "++" || previousId == "--"
            || previousId == "+" || previousId == "-" || previousId == "&"
            || previousId == "!" || previousId == "~"
            || previousId == "=" || previousId == "+=" || previousId == "-="
            || previousId == "*=" || previousId == "/=" || previousId == "%="
            || previousId == "<<=" || previousId == ">>=" || previousId == "&="
            || previousId == "^=" || previousId == "|="
            || previousId == "==" || previousId == "!=" || previousId == "<"
            || previousId == ">" || previousId == "<=" || previousId == ">="
            || previousId == "<<" || previousId == ">>" || previousId == "/"
            || previousId == "%" || previousId == "^" || previousId == "|"
            || previousId == "&&" || previousId == "||" || previousId == "?"
            // Do not include ',' (parameter/decl lists need typedef types) or ':'
            // (labels may be followed by a declaration starting with a typedef).
            || previousId == "["
            || previousId == "id" || previousId == "int_const" || previousId == "char_const"
            || previousId == "float_const" || previousId == "string" || previousId == "enumeration_const";
}

scanner::Token reclassifyToken(const scanner::Token& token, const std::string& previousId) {
    if (token.id != "id" && token.id != "typedef_name") {
        return token;
    }
    // Object declarators that reuse a typedef name force later uses to "id".
    if (scanner::TypedefRegistry::isIdentifierShadow(token.lexeme)) {
        return scanner::Token { "id", token.lexeme, token.context };
    }
    if (!scanner::TypedefRegistry::has(token.lexeme)) {
        if (token.id == "typedef_name") {
            return scanner::Token { "id", token.lexeme, token.context };
        }
        return token;
    }
    if (!previousId.empty() && preferIdentifierOverTypedef(previousId)) {
        return scanner::Token { "id", token.lexeme, token.context };
    }
    return scanner::Token { "typedef_name", token.lexeme, token.context };
}

} // namespace

TokenStream::TokenStream(std::function<scanner::Token()> scan) :
    scan { scan },
    currentToken { this->scan() }
{
}

scanner::Token TokenStream::getCurrentToken() const {
    const scanner::Token& raw = forgedToken ? *forgedToken : *currentToken;
    return reclassifyToken(raw, previousTokenId);
}

scanner::Token TokenStream::nextToken() {
    previousTokenId = getCurrentToken().id;
    // Brace scopes bound typedef-name object shadows (see TypedefRegistry).
    if (previousTokenId == "{") {
        scanner::TypedefRegistry::pushIdentifierShadowScope();
    } else if (previousTokenId == "}") {
        scanner::TypedefRegistry::popIdentifierShadowScope();
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
