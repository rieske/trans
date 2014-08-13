#ifndef _A_EXPRESSIONS_NODE_H_
#define _A_EXPRESSIONS_NODE_H_

#include <memory>
#include <string>
#include <vector>

#include "AssignmentExpression.h"

namespace semantic_analyzer {

class AssignmentExpressionList: public AbstractSyntaxTreeNode {
public:
    AssignmentExpressionList();
	AssignmentExpressionList(std::unique_ptr<Expression> expression);

	void addExpression(std::unique_ptr<Expression> expression);
	const std::vector<std::unique_ptr<Expression>>& getExpressions() const;

	void accept(AbstractSyntaxTreeVisitor& visitor) override;

	static const std::string ID;

private:
	vector<std::unique_ptr<Expression>> expressions;
};

}

#endif // _A_EXPRESSIONS_NODE_H_
