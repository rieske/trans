#ifndef OPERATOR_H_
#define OPERATOR_H_

#include <string>

#include "AbstractSyntaxTreeNode.h"

namespace ast {

class Operator: public AbstractSyntaxTreeNode {
public:
    explicit Operator(std::string lexeme);
    virtual ~Operator() = default;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    std::string getLexeme() const;

private:
    std::string lexeme;
};

} /* namespace ast */

#endif /* OPERATOR_H_ */
