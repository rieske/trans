#ifndef INITIALIZERLISTEXPRESSION_H_
#define INITIALIZERLISTEXPRESSION_H_

#include <memory>
#include <vector>

#include "DesignatorStep.h"
#include "Expression.h"

namespace ast {

// Brace-enclosed initializer list: { e1, e2, ... } (and the trailing-comma form),
// including designated forms { .field = e, [i] = e }.
class InitializerListExpression: public Expression {
public:
    explicit InitializerListExpression(std::vector<InitializerElement> elements);
    virtual ~InitializerListExpression() = default;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    std::optional<type::Type> typeAtParseTime(const ParseEnvironment& environment) const override;

    translation_unit::Context getContext() const override;

    const std::vector<InitializerElement>& getElements() const;
    void visitElements(AbstractSyntaxTreeVisitor& visitor);
    void appendElement(InitializerElement element);

private:
    std::vector<InitializerElement> elements;
};

} // namespace ast

#endif // INITIALIZERLISTEXPRESSION_H_
