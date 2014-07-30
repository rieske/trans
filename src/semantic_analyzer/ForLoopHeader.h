#ifndef FORLOOPHEADER_H_
#define FORLOOPHEADER_H_

#include <memory>

#include "LoopHeader.h"

namespace semantic_analyzer {

class Expression;

class ForLoopHeader: public LoopHeader {
public:
	ForLoopHeader(std::unique_ptr<Expression> initialization, std::unique_ptr<Expression> clause, std::unique_ptr<Expression> increment,
			SymbolTable *st);
	virtual ~ForLoopHeader();

	void accept(const AbstractSyntaxTreeVisitor& visitor) const override;

	const std::unique_ptr<Expression> initialization;
	const std::unique_ptr<Expression> clause;
	const std::unique_ptr<Expression> increment;
};

} /* namespace semantic_analyzer */

#endif /* FORLOOPHEADER_H_ */