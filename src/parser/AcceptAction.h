#ifndef ACCEPTACTION_H_
#define ACCEPTACTION_H_

#include <memory>
#include <stack>
#include <string>

#include "../semantic_analyzer/SemanticAnalyzer.h"
#include "Action.h"
#include "ParsingTable.h"
#include "TokenStream.h"

class AcceptAction: public Action {
public:
	AcceptAction();
	virtual ~AcceptAction();

	std::unique_ptr<SyntaxTree> perform(std::stack<parse_state>&, TokenStream&, SemanticAnalyzer& semanticAnalyzer) const override;

	std::string serialize() const override;
};

#endif /* ACCEPTACTION_H_ */
