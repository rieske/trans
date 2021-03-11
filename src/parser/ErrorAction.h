#ifndef ERRORACTION_H_
#define ERRORACTION_H_

#include <memory>
#include <stack>
#include <string>

#include "Action.h"
#include "TokenStream.h"

namespace parser {

class ErrorAction: public Action {
public:
	ErrorAction(parse_state state, std::string forgeToken, int expectedSymbol, const Grammar* grammar);
	virtual ~ErrorAction();

	bool parse(std::stack<parse_state>& parsingStack, TokenStream& tokenStream, std::unique_ptr<SyntaxTreeBuilder>& syntaxTreeBuilder) const override;

	std::string serialize() const override;

private:
	const parse_state state;

	std::string forgeToken;
	int expectedSymbol;

    const Grammar* grammar;
};

} // namespace parser

#endif // ERRORACTION_H_
