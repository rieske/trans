#ifndef _ACTION_H_
#define _ACTION_H_

#include <memory>
#include <stack>
#include <string>

#include "../semantic_analyzer/SemanticAnalyzer.h"
#include "LookaheadActionTable.h"
#include "ParsingTable.h"
#include "TokenStream.h"

namespace parser {

class Action {
public:
	virtual ~Action() {
	}

	virtual bool parse(std::stack<parse_state>& parsingStack, TokenStream& tokenStream, SemanticAnalyzer& semanticAnalyzer) const = 0;

	virtual std::string serialize() const = 0;

	static std::unique_ptr<Action> deserialize(std::string serializedAction, const ParsingTable& parsingTable, const Grammar& grammar);

private:
	static LR1Item getReductionById(std::string nonterminalId, size_t productionId, const Grammar& grammar);
};

}

#endif // _ACTION_H_
