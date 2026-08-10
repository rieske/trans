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

    void forgeToken(scanner::Token forgedToken);
    bool currentTokenIsForged() const;

private:
    void advanceIdContext(const scanner::Token& token);
    scanner::Token reclassify(const scanner::Token& token) const;

    std::function<scanner::Token()> scan;
    scanner::LexicalSession& session_;

    std::optional<const scanner::Token> currentToken;
    std::optional<const scanner::Token> forgedToken;
    LexIdContext idContext_ { LexIdContext::AsType };
};

} // namespace parser

#endif // TOKENSTREAM_H_
