#ifndef ERRORACTION_H_
#define ERRORACTION_H_

#include <stack>
#include <string>

#include "../semantic_analyzer/SemanticAnalyzer.h"
#include "Action.h"
#include "LookaheadActionTable.h"
#include "TokenStream.h"

class ErrorAction: public Action {
public:
	ErrorAction(parse_state state, std::string forgeToken, std::string expectedSymbol);
	virtual ~ErrorAction();

	bool parse(std::stack<parse_state>& parsingStack, TokenStream& tokenStream, SemanticAnalyzer& semanticAnalyzer) const override;

	std::string serialize() const override;

private:
	const parse_state state;

	std::string forgeToken;
	std::string expectedSymbol;
};

#endif /* ERRORACTION_H_ */
