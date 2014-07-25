#ifndef LOOPSTATEMENT_H_
#define LOOPSTATEMENT_H_

#include <memory>

#include "NonterminalNode.h"

namespace semantic_analyzer {

class LoopHeader;

class LoopStatement: public NonterminalNode {
public:
	LoopStatement(std::unique_ptr<LoopHeader> header, std::unique_ptr<AbstractSyntaxTreeNode> body, SymbolTable *st);
	virtual ~LoopStatement();

	void accept(const AbstractSyntaxTreeVisitor& visitor) const override;

	const std::unique_ptr<LoopHeader> header;
	const std::unique_ptr<AbstractSyntaxTreeNode> body;
};

} /* namespace semantic_analyzer */

#endif /* LOOPSTATEMENT_H_ */
