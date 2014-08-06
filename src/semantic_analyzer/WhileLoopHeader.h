#ifndef WHILELOOPHEADER_H_
#define WHILELOOPHEADER_H_

#include <memory>

#include "LoopHeader.h"

namespace semantic_analyzer {

class Expression;

class WhileLoopHeader: public LoopHeader {
public:
	WhileLoopHeader(std::unique_ptr<Expression> clause, SymbolTable *st);
	virtual ~WhileLoopHeader();

	void accept(AbstractSyntaxTreeVisitor& visitor) const override;

	const std::unique_ptr<Expression> clause;
};

} /* namespace semantic_analyzer */

#endif /* WHILELOOPHEADER_H_ */
