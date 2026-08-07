#ifndef _IDENTIFIER_EXPRESSION_H_
#define _IDENTIFIER_EXPRESSION_H_

#include <optional>
#include <string>

#include "Expression.h"

namespace ast {

class IdentifierExpression: public Expression {
public:
    IdentifierExpression(std::string identifier, translation_unit::Context context);
    virtual ~IdentifierExpression();

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    translation_unit::Context getContext() const override;
    std::string getIdentifier() const;

    // Parse-time const-fold residual for enumerators (not AnnotationStore).
    // SA may clear when an ordinary object shadows the name, or re-set from
    // the symbol table; codegen may read the final fold after SA.
    void setFoldedConstant(long value);
    // Drop a parse-time fold when SA binds this name to an object (shadow).
    void clearFoldedConstant();
    bool hasFoldedConstant() const;
    long getFoldedConstant() const;
    bool evaluateConstant(long& value) const override;

    // Rodata label for __func__ (same CG path as string literals).
    void setStringConstantLabel(std::string label);
    bool hasStringConstantLabel() const;
    const std::string& getStringConstantLabel() const;

private:
    std::string identifier;
    translation_unit::Context context;
    std::optional<long> foldedConstant;
    std::optional<std::string> stringConstantLabel;
};

} // namespace ast

#endif // _IDENTIFIER_EXPRESSION_H_
