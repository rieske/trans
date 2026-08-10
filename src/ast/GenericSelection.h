#ifndef GENERICSELECTION_H_
#define GENERICSELECTION_H_

#include <memory>
#include <optional>
#include <vector>

#include "Expression.h"
#include "TypeSpecifier.h"

namespace ast {

struct GenericAssociation {
    std::optional<TypeSpecifier> typeName;
    std::unique_ptr<Expression> expression;

    bool isDefault() const { return !typeName.has_value(); }
};

class GenericSelection: public Expression {
public:
    GenericSelection(translation_unit::Context context,
            std::unique_ptr<Expression> controlling,
            std::vector<GenericAssociation> associations);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    translation_unit::Context getContext() const override;
    bool evaluateConstant(long& value) const override;

    Expression& controllingExpression() { return *controlling_; }
    const Expression& controllingExpression() const { return *controlling_; }
    std::vector<GenericAssociation>& associations() { return associations_; }
    const std::vector<GenericAssociation>& associations() const { return associations_; }

    void select(std::size_t index, symbols::AnnotationStore& store);
    bool hasSelected() const { return selectedIndex_.has_value(); }
    Expression& selectedExpression();
    const Expression& selectedExpression() const;

private:
    translation_unit::Context context_;
    std::unique_ptr<Expression> controlling_;
    std::vector<GenericAssociation> associations_;
    std::optional<std::size_t> selectedIndex_;
};

} // namespace ast

#endif // GENERICSELECTION_H_
