#ifndef INITIALIZERLISTEXPRESSION_H_
#define INITIALIZERLISTEXPRESSION_H_

#include <memory>
#include <vector>

#include "Expression.h"

namespace ast {

// Brace-enclosed list: { e1, e2, ... } (no designators in this slice).
class InitializerListExpression: public Expression {
public:
    explicit InitializerListExpression(std::vector<std::unique_ptr<Expression>> elements);
    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    translation_unit::Context getContext() const override;
    const std::vector<std::unique_ptr<Expression>>& getElements() const;
    void visitElements(AbstractSyntaxTreeVisitor& visitor);

private:
    std::vector<std::unique_ptr<Expression>> elements;
};

} // namespace ast

#endif
