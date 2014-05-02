#ifndef PARSETREEBUILDER_H_
#define PARSETREEBUILDER_H_

#include "semantic_analyzer/SyntaxTreeBuilder.h"

class ParseTreeBuilder: public SyntaxTreeBuilder {
public:
	ParseTreeBuilder();
	virtual ~ParseTreeBuilder();
};

#endif /* PARSETREEBUILDER_H_ */
