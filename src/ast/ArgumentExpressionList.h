#ifndef _A_EXPRESSIONS_NODE_H_
#define _A_EXPRESSIONS_NODE_H_

#include <memory>
#include <string>
#include <vector>

#include "AssignmentExpression.h"

namespace ast {

class ArgumentExpressionList: public AbstractSyntaxTreeNode {
public:
    ArgumentExpressionList();
	ArgumentExpressionList(std::unique_ptr<Expression> expression);

	// FIXME: make immutable
	void addExpression(std::unique_ptr<Expression> expression);
	const std::vector<std::unique_ptr<Expression>>& getExpressions() const;

	void accept(AbstractSyntaxTreeVisitor& visitor) override;

	static const std::string ID;

private:
	std::vector<std::unique_ptr<Expression>> expressions;
};

}

#endif // _A_EXPRESSIONS_NODE_H_
