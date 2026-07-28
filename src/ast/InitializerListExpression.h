#ifndef INITIALIZERLISTEXPRESSION_H_
#define INITIALIZERLISTEXPRESSION_H_

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "Expression.h"

namespace ast {

// One segment of a C99 designator list: .name or [n].
struct DesignatorStep {
    enum class Kind { Member, Index };
    Kind kind { Kind::Member };
    std::string memberName;
    // Folded index when known; indexExpression retained for delayed fold (e.g. sizeof after SA).
    std::optional<long> index;
    std::unique_ptr<Expression> indexExpression;

    static DesignatorStep member(std::string name) {
        DesignatorStep s;
        s.kind = Kind::Member;
        s.memberName = std::move(name);
        return s;
    }

    static DesignatorStep indexWithExpression(std::unique_ptr<Expression> expr) {
        DesignatorStep s;
        s.kind = Kind::Index;
        long v = 0;
        if (expr && expr->evaluateConstant(v)) {
            s.index = v;
        } else {
            s.indexExpression = std::move(expr);
        }
        return s;
    }
};

// One element of a brace initializer, optionally with a C99 designator list.
struct InitializerElement {
    std::unique_ptr<Expression> value;
    // Empty = positional. Otherwise ordered designator steps (.a[1].b ...).
    std::vector<DesignatorStep> designator;

    explicit InitializerElement(std::unique_ptr<Expression> value)
            : value { std::move(value) } {
    }

    bool isDesignated() const { return !designator.empty(); }
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

private:
    std::vector<InitializerElement> elements;
};

} // namespace ast

#endif // INITIALIZERLISTEXPRESSION_H_
