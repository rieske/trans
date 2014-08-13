#ifndef FORLOOPHEADER_H_
#define FORLOOPHEADER_H_

#include <memory>

#include "LoopHeader.h"

namespace semantic_analyzer {

class Expression;

class ForLoopHeader: public LoopHeader {
public:
	ForLoopHeader(std::unique_ptr<Expression> initialization, std::unique_ptr<Expression> clause, std::unique_ptr<Expression> increment);
	virtual ~ForLoopHeader();

	void accept(AbstractSyntaxTreeVisitor& visitor) override;

	const std::unique_ptr<Expression> initialization;
	const std::unique_ptr<Expression> clause;
};

} /* namespace semantic_analyzer */

#endif /* FORLOOPHEADER_H_ */
