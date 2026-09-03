#ifndef _LR1PARSER_H_
#define _LR1PARSER_H_

#include <memory>
#include <optional>
#include <string>

#include "Parser.h"

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

class LR1Parser: public Parser {
public:
	explicit LR1Parser(const ParsingTable& parsingTable);
	LR1Parser(const ParsingTable&&) = delete;
	virtual ~LR1Parser();

	bool parse(scanner::Scanner& scanner, SyntaxTreeBuilder& syntaxTreeBuilder) override;
private:
	const ParsingTable& parsingTable;
};

} // namespace parser

#endif // _LR1PARSER_H_
