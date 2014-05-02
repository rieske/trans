#ifndef SEMANTICSYNTAXTREEBUILDER_H_
#define SEMANTICSYNTAXTREEBUILDER_H_

#include <string>
#include <vector>

#include "param_decl_node.h"
#include "SyntaxTreeBuilder.h"

class Token;

class SemanticSyntaxTreeBuilder: public SyntaxTreeBuilder {
public:
	SemanticSyntaxTreeBuilder();
	virtual ~SemanticSyntaxTreeBuilder();

	void makeTerminalNode(string terminal, Token token);
	void makeNonTerminalNode(std::string left, int childrenCount, std::string reduction);

private:
	void adjustScope(std::string lexeme);

	vector<ParamDeclNode *> declaredParams;
};

#endif /* SEMANTICSYNTAXTREEBUILDER_H_ */
