#ifndef _LR1PARSER_H_
#define _LR1PARSER_H_

#include <optional>
#include <string>

namespace scanner {
class Scanner;
}

namespace parser {

class ParsingTable;
class TokenStream;
class SyntaxTreeBuilder;
class ParseExtensions;

enum class LrFinish { Complete, Stopped };

struct LrStop {
    int definingSymbol;
    std::string lookahead;
    const bool* live { nullptr };
};

LrFinish runLrParse(const ParsingTable& parsingTable, TokenStream& tokenStream,
        SyntaxTreeBuilder& syntaxTreeBuilder, ParseExtensions* extensions = nullptr,
        std::optional<LrStop> stop = std::nullopt);

class LR1Parser {
public:
	explicit LR1Parser(const ParsingTable& parsingTable);
	LR1Parser(const ParsingTable&&) = delete;

	bool parse(scanner::Scanner& scanner, SyntaxTreeBuilder& syntaxTreeBuilder) const;
private:
	const ParsingTable& parsingTable;
};

} // namespace parser

#endif // _LR1PARSER_H_
