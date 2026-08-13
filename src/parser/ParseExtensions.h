#ifndef PARSE_EXTENSIONS_H_
#define PARSE_EXTENSIONS_H_

#include <cstddef>
#include <optional>

#include "scanner/Token.h"

namespace parser {

class ParsingTable;
class TokenStream;
class SyntaxTreeBuilder;

// Optional dialect hook for the LR driver. Null means ISO-only.
// The product grammar is ISO C; GNU forms live here, not in grammar.bnf.
class ParseExtensions {
public:
    virtual ~ParseExtensions() = default;

    virtual std::optional<std::size_t> tryGoto(std::size_t state, TokenStream& tokenStream,
            const ParsingTable& parsingTable) = 0;
    virtual bool accept(TokenStream& tokenStream, const ParsingTable& parsingTable,
            SyntaxTreeBuilder& syntaxTreeBuilder) = 0;

    // True when the current token is a type-spec extension spelled as `id`
    // (e.g. __int128). The LR driver may probe a FIRST(<type_spec>) terminal
    // reduce and retry when the pure-reduce FOLLOW set omits `id`.
    virtual bool isTypeExtensionToken(const scanner::Token& token) const {
        (void)token;
        return false;
    }
};

} // namespace parser

#endif
