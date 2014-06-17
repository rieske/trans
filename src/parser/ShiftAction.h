#ifndef SHIFTACTION_H_
#define SHIFTACTION_H_

#include <memory>
#include <stack>
#include <string>

#include "../semantic_analyzer/SemanticAnalyzer.h"
#include "Action.h"
#include "ParsingTable.h"
#include "TokenStream.h"

class ShiftAction: public Action {
public:
	ShiftAction(parse_state state);
	virtual ~ShiftAction();

	std::unique_ptr<SyntaxTree> perform(std::stack<parse_state>& parsingStack, TokenStream& tokenStream,
			SemanticAnalyzer& semanticAnalyzer) const override;

	std::string describe() const override;

private:
	const parse_state state;
};

#endif /* SHIFTACTION_H_ */
