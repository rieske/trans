#ifndef SHIFTACTION_H_
#define SHIFTACTION_H_

#include <stack>
#include <string>

#include "semantic_analyzer/SemanticAnalyzer.h"
#include "Action.h"
#include "LookaheadActionTable.h"
#include "TokenStream.h"

namespace parser {

class ShiftAction: public Action {
public:
	ShiftAction(parse_state state);
	virtual ~ShiftAction();

	bool parse(std::stack<parse_state>& parsingStack, TokenStream& tokenStream, std::unique_ptr<SyntaxTreeBuilder>& syntaxTreeBuilder) const override;

	std::string serialize() const override;

private:
	const parse_state state;
};

}

#endif /* SHIFTACTION_H_ */
