#ifndef SYNTAXTREEBUILDER_H_
#define SYNTAXTREEBUILDER_H_

#include <memory>
#include <string>

#include "Production.h"

class TranslationUnitContext;

namespace parser {

class SyntaxTree;

class SyntaxTreeBuilder {
public:
	virtual ~SyntaxTreeBuilder() {}

	virtual std::unique_ptr<SyntaxTree> build() = 0;

	virtual void makeTerminalNode(std::string type, std::string value, const TranslationUnitContext& context) = 0;
	virtual void makeNonterminalNode(std::string definingSymbol, Production production) = 0;
};

} /* namespace parser */

#endif /* SYNTAXTREEBUILDER_H_ */
