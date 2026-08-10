#ifndef TOKENSTREAM_H_
#define TOKENSTREAM_H_

#include "scanner/Token.h"

#include <functional>
#include <optional>

namespace scanner {
class LexicalSession;
}

namespace parser {

enum class LexIdContext {
    AsType,
    AsIdentifier,
};

class TokenStream {
public:
    TokenStream(std::function<scanner::Token()> scan, scanner::LexicalSession& session);

    scanner::Token getCurrentToken() const;
    scanner::Token nextToken();
    scanner::Token peek();
    // Advance without session or id-context effects. Returns the raw current token.
    scanner::Token takeRaw();
    // Make token current; previous current becomes peek. No session effects.
    void unget(scanner::Token token);

    void forgeToken(scanner::Token forgedToken);
    bool currentTokenIsForged() const;

private:
    void advanceIdContext(const scanner::Token& token);
    scanner::Token reclassify(const scanner::Token& token) const;

    std::function<scanner::Token()> scan;
    scanner::LexicalSession& session_;

    std::optional<const scanner::Token> currentToken;
    std::optional<const scanner::Token> forgedToken;
    std::optional<const scanner::Token> lookahead_;
    LexIdContext idContext_ { LexIdContext::AsType };
};

} // namespace parser

#endif // TOKENSTREAM_H_
