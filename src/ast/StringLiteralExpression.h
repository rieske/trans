#ifndef AST_STRING_LITERAL_EXPRESSION_H_
#define AST_STRING_LITERAL_EXPRESSION_H_

#include <string>

#include "translation_unit/Context.h"
#include "Expression.h"

namespace ast {

class StringLiteralExpression: public Expression {
public:
    StringLiteralExpression(std::string value, translation_unit::Context context);
    virtual ~StringLiteralExpression();

    translation_unit::Context getContext() const override;
    std::string getValue() const;

    void setRodataLabel(symbols::AnnotationStore& store, std::string label);
    const std::string* rodataLabel(const symbols::AnnotationStore& store) const;

    std::optional<type::Type> typeAtParseTime(const ParseEnvironment& environment) const override;
    void accept(AbstractSyntaxTreeVisitor& visitor) override;

private:
    std::string value;
    translation_unit::Context context;
};

} // namespace ast

#endif // AST_STRING_LITERAL_EXPRESSION_H_
