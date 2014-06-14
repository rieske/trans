#ifndef REDUCEACTION_H_
#define REDUCEACTION_H_

#include <map>
#include <memory>
#include <stack>
#include <string>
#include <unordered_map>

#include "../semantic_analyzer/SemanticAnalyzer.h"
#include "Action.h"
#include "ParsingTable.h"
#include "TokenStream.h"

class LR1Item;

class ReduceAction: public Action {
public:
	ReduceAction(LR1Item reduction,
			const std::unordered_map<parse_state, std::map<std::shared_ptr<const GrammarSymbol>, parse_state>>* gotoTable);
	virtual ~ReduceAction();

	std::unique_ptr<SyntaxTree> perform(std::stack<parse_state>& parsingStack, TokenStream& tokenStream,
			SemanticAnalyzer& semanticAnalyzer) const override;

	std::string describe() const override;

private:
	std::unique_ptr<const LR1Item> reduction;

	const std::unordered_map<parse_state, std::map<std::shared_ptr<const GrammarSymbol>, parse_state>>* gotoTable;
};

#endif /* REDUCEACTION_H_ */
