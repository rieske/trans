#ifndef REDUCEACTION_H_
#define REDUCEACTION_H_

#include <memory>
#include <stack>

#include "../semantic_analyzer/SemanticAnalyzer.h"
#include "../util/Logger.h"
#include "Action.h"
#include "ParsingTable.h"
#include "TokenStream.h"

class ReduceAction: public Action {
public:
	ReduceAction(LR1Item reduction, const std::map<parse_state, std::map<std::shared_ptr<const GrammarSymbol>, parse_state>>* gotoTable,
			Logger logger);
	virtual ~ReduceAction();

	std::unique_ptr<SyntaxTree> perform(std::stack<parse_state>& parsingStack, TokenStream& tokenStream, SemanticAnalyzer& semanticAnalyzer)
			override;

	std::string describe() const override;

private:
	std::unique_ptr<const LR1Item> reduction;

	const std::map<parse_state, std::map<std::shared_ptr<const GrammarSymbol>, parse_state>>* gotoTable;
};

#endif /* REDUCEACTION_H_ */
