#ifndef _ACTION_H_
#define _ACTION_H_

#include <memory>
#include <stack>
#include <string>

#include "../semantic_analyzer/SemanticAnalyzer.h"
#include "../util/Logger.h"
#include "ParsingTable.h"
#include "TokenStream.h"

class Action {
public:
	Action(Logger logger);
	virtual ~Action();

	virtual std::unique_ptr<SyntaxTree> perform(std::stack<parse_state>& parsingStack, TokenStream& tokenStream,
			SemanticAnalyzer& semanticAnalyzer) = 0;

	virtual std::string describe() const = 0;

protected:
	Logger logger;
};

#endif // _ACTION_H_
