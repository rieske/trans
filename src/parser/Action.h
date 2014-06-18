#ifndef _ACTION_H_
#define _ACTION_H_

#include <map>
#include <memory>
#include <stack>
#include <string>
#include <unordered_map>

#include "../semantic_analyzer/SemanticAnalyzer.h"
#include "ParsingTable.h"
#include "TokenStream.h"

class Grammar;

class Action {
public:
	virtual ~Action() {
	}

	virtual std::unique_ptr<SyntaxTree> perform(std::stack<parse_state>& parsingStack, TokenStream& tokenStream,
			SemanticAnalyzer& semanticAnalyzer) const = 0;

	virtual std::string serialize() const = 0;

	static std::unique_ptr<Action> deserialize(std::string& serializedAction, const Grammar& grammar,
			const std::unordered_map<parse_state, std::map<std::shared_ptr<const GrammarSymbol>, parse_state>>* gotoTable);
};

#endif // _ACTION_H_
