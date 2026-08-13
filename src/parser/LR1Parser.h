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

// Subparse stop policy for runLrParse. Prefer the factories; do not mix modes.
// Nest tracks (), [], and {} for every mode so compound-literal commas stay inside.
struct LrStop {
    enum class Mode {
        // Reduce definingSymbol when the live lookahead equals `lookahead`.
        Lookahead,
        // definingSymbol is finished: reduce when it cannot grow further, and
        // stop before shifting any assignment operator (C unary vs assign).
        Complete,
    };

    int definingSymbol;
    Mode mode;
    const bool* live { nullptr };
    // Set only for Mode::Lookahead; nullopt for Mode::Complete.
    std::optional<std::string> lookahead;

    bool isComplete() const { return mode == Mode::Complete; }

    static LrStop untilLookahead(int definingSymbol, std::string lookahead,
            const bool* live = nullptr);
    static LrStop untilComplete(int definingSymbol, const bool* live = nullptr);
};

LrFinish runLrParse(const ParsingTable& parsingTable, TokenStream& tokenStream,
        SyntaxTreeBuilder& syntaxTreeBuilder, ParseExtensions* extensions = nullptr,
        std::optional<LrStop> stop = std::nullopt);

class LR1Parser: public Parser {
public:
	explicit LR1Parser(const ParsingTable& parsingTable);
	LR1Parser(const ParsingTable&&) = delete;
	virtual ~LR1Parser();

	std::unique_ptr<SyntaxTree> parse(scanner::Scanner& scanner, SyntaxTreeBuilder& syntaxTreeBuilder) override;
private:
	const ParsingTable& parsingTable;
};

} // namespace parser

#endif // _LR1PARSER_H_
