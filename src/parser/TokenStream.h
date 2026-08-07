#ifndef TOKENSTREAM_H_
#define TOKENSTREAM_H_

#include "scanner/Token.h"

#include <functional>
#include <optional>

namespace scanner {
class TypedefRegistry;
}

namespace parser {

// Role the next identifier plays when its spelling collides with a typedef name.
// Two-state previous-token machine: transitions live only in roleAfter (TokenStream.cpp).
// Approximate C (brace-matched shadows on TypedefRegistry); not full type vs expression
// position. Primitive type-specs force AsIdentifier (`int T`); qualifiers keep type
// (`const foo_t`). Parser-fed context is the larger redesign if this limit is hit.
enum class LexIdContext {
    AsType,        // default: typedef spelling stays typedef_name
    AsIdentifier,  // object / declarator / member / tag / expression name
};

class TokenStream {
public:
    TokenStream(std::function<scanner::Token()> scan, scanner::TypedefRegistry& typedefs);

    // Live view: reclassifies against current registry (do not cache .id across
    // reduce-time typedef/shadow writes).
    scanner::Token getCurrentToken() const;
    scanner::Token nextToken();

    void forgeToken(scanner::Token forgedToken);
    bool currentTokenIsForged() const;

    LexIdContext idContext() const { return idContext_; }

private:
    void advanceIdContext(const scanner::Token& token);
    scanner::Token reclassify(const scanner::Token& token) const;

    std::function<scanner::Token()> scan;
    scanner::TypedefRegistry& typedefs_;

    std::optional<const scanner::Token> currentToken;
    std::optional<const scanner::Token> forgedToken;
    LexIdContext idContext_ { LexIdContext::AsType };
};

} // namespace parser

#endif // TOKENSTREAM_H_
