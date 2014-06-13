#ifndef ACCEPTACTION_H_
#define ACCEPTACTION_H_

#include <iostream>
#include <memory>
#include <stack>

#include "../semantic_analyzer/SemanticAnalyzer.h"
#include "../util/Logger.h"
#include "Action.h"
#include "ParsingTable.h"
#include "TokenStream.h"

class AcceptAction: public Action {
public:
	AcceptAction(Logger logger);
	virtual ~AcceptAction();

	std::unique_ptr<SyntaxTree> perform(std::stack<parse_state>&, TokenStream&, SemanticAnalyzer& semanticAnalyzer) override;

	std::string describe() const override;
};

#endif /* ACCEPTACTION_H_ */
