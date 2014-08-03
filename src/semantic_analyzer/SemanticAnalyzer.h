#ifndef SYNTAXTREEBUILDERDECORATOR_H_
#define SYNTAXTREEBUILDERDECORATOR_H_

#include "../parser/ParseTree.h"

namespace semantic_analyzer {

class AbstractSyntaxTree;

class SemanticAnalyzer {
public:
	SemanticAnalyzer();
	virtual ~SemanticAnalyzer();

	void analyze(const parser::SyntaxTree& syntaxTree);
};

}

#endif /* SYNTAXTREEBUILDERDECORATOR_H_ */
