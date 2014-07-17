#ifndef SYNTAXTREEBUILDER_H_
#define SYNTAXTREEBUILDER_H_

#include <memory>
#include <string>

#include "Production.h"

class Token;

namespace parser {

class SyntaxTree;

class SyntaxTreeBuilder {
public:
	virtual ~SyntaxTreeBuilder() {}

	virtual std::unique_ptr<SyntaxTree> build() = 0;

	virtual void makeTerminalNode(const Token& token) = 0;
	virtual void makeNonterminalNode(std::string definingSymbol, parser::Production production) = 0;
};

} /* namespace parser */

#endif /* SYNTAXTREEBUILDER_H_ */
