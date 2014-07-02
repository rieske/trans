#ifndef REDUCEACTION_H_
#define REDUCEACTION_H_

#include <memory>
#include <stack>
#include <string>

#include "../semantic_analyzer/SemanticAnalyzer.h"
#include "Action.h"
#include "LookaheadActionTable.h"
#include "TokenStream.h"

class LR1Item;

class ReduceAction: public Action {
public:
	ReduceAction(LR1Item reduction, const ParsingTable* parsingTable);
	virtual ~ReduceAction();

	bool parse(std::stack<parse_state>& parsingStack, TokenStream& tokenStream, SemanticAnalyzer& semanticAnalyzer) const override;

	std::string serialize() const override;

private:
	std::unique_ptr<const LR1Item> reduction;

	const ParsingTable* parsingTable;
};

#endif /* REDUCEACTION_H_ */
