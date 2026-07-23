#ifndef INITIALIZERLISTEXPRESSION_H_
#define INITIALIZERLISTEXPRESSION_H_

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "Expression.h"

namespace ast {

// One element of a brace initializer, optionally with a C99 designator (.member or [index]).
struct InitializerElement {
    std::unique_ptr<Expression> value;
    // Nested designator path: .a.b.c stores {"a","b","c"}. Empty = positional.
    std::vector<std::string> memberPath;
    std::optional<long> arrayIndex; // [n] when folded as a constant

    explicit InitializerElement(std::unique_ptr<Expression> value)
            : value { std::move(value) } {
    }
};

// Brace-enclosed initializer list: { e1, e2, ... } (and the trailing-comma form),
// including designated forms { .field = e, [i] = e }.
class InitializerListExpression: public Expression {
public:
    explicit InitializerListExpression(std::vector<InitializerElement> elements);
    virtual ~InitializerListExpression() = default;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    translation_unit::Context getContext() const override;

    const std::vector<InitializerElement>& getElements() const;
    void visitElements(AbstractSyntaxTreeVisitor& visitor);
    void appendElement(InitializerElement element);

    // Fold every element as an integer constant expression (no designators).
    bool evaluateConstantElements(std::vector<long>& values) const;

private:
    std::vector<InitializerElement> elements;
};

} // namespace ast

#endif // INITIALIZERLISTEXPRESSION_H_
