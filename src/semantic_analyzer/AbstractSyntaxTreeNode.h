#ifndef ABSTRACTSYNTAXTREENODE_H_
#define ABSTRACTSYNTAXTREENODE_H_

#include <vector>

#include "../code_generator/quadruple.h"

namespace semantic_analyzer {

class AbstractSyntaxTreeVisitor;

class AbstractSyntaxTreeNode {
public:
	AbstractSyntaxTreeNode();
	virtual ~AbstractSyntaxTreeNode();

	virtual void accept(AbstractSyntaxTreeVisitor& visitor) const;

	//FIXME:
	virtual std::vector<Quadruple*> getCode() const = 0;
};

} /* namespace semantic_analyzer */

#endif /* ABSTRACTSYNTAXTREENODE_H_ */
