#ifndef _ACTION_H_
#define _ACTION_H_

#include <memory>
#include <stack>
#include <string>

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

	static std::unique_ptr<Action> deserialize(std::string serializedAction, const Grammar& grammar,
			const ParsingTable* parsingTable);
};

#endif // _ACTION_H_
