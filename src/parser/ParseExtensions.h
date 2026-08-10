#ifndef PARSE_EXTENSIONS_H_
#define PARSE_EXTENSIONS_H_

#include <cstddef>
#include <optional>

namespace parser {

class ParsingTable;
class TokenStream;
class SyntaxTreeBuilder;

// Optional dialect hook for the LR driver. Null means ISO-only.
class ParseExtensions {
public:
    virtual ~ParseExtensions() = default;

    virtual std::optional<std::size_t> tryGoto(std::size_t state, TokenStream& tokenStream,
            const ParsingTable& parsingTable) = 0;
    virtual bool accept(TokenStream& tokenStream, const ParsingTable& parsingTable,
            SyntaxTreeBuilder& syntaxTreeBuilder) = 0;
};

} // namespace parser

#endif
