#ifndef PARSETREEBUILDER_H_
#define PARSETREEBUILDER_H_

#include <string>
#include <vector>

#include "../parser/node.h"
#include "SyntaxTreeBuilder.h"

class ParseTreeBuilder: public SyntaxTreeBuilder {
public:
	ParseTreeBuilder();
	virtual ~ParseTreeBuilder();

	void makeTerminalNode(string terminal, Token token);
	void makeNonTerminalNode(std::string left, int childrenCount, std::string reduction);
};

#endif /* PARSETREEBUILDER_H_ */
