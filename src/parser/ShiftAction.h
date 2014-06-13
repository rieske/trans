#ifndef SHIFTACTION_H_
#define SHIFTACTION_H_

#include "../util/Logger.h"
#include "Action.h"
#include "ParsingTable.h"

class ShiftAction: public Action {
public:
	ShiftAction(parse_state state, Logger logger);
	virtual ~ShiftAction();

	std::unique_ptr<SyntaxTree> perform(std::stack<parse_state>& parsingStack, TokenStream& tokenStream,
			SemanticAnalyzer& semanticAnalyzer) override;

	std::string describe() const override;

private:
	const parse_state state;
};

#endif /* SHIFTACTION_H_ */
