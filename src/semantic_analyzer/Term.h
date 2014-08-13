#ifndef _TERM_NODE_H_
#define _TERM_NODE_H_

#include <string>

#include "Expression.h"
#include "TerminalSymbol.h"

namespace semantic_analyzer {

class Term: public Expression {
public:
	Term(TerminalSymbol term);
	virtual ~Term();

	void accept(AbstractSyntaxTreeVisitor& visitor) override;

	static const std::string ID;

	const TerminalSymbol term;
};

}

#endif // _TERM_NODE_H_
