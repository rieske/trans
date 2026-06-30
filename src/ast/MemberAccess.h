#ifndef MEMBERACCESS_H_
#define MEMBERACCESS_H_

#include <memory>
#include <string>

#include "Expression.h"
#include "translation_unit/Context.h"

namespace ast {

// postfix . id  or  postfix -> id
class MemberAccess: public Expression {
public:
    MemberAccess(std::unique_ptr<Expression> base, std::string memberName, bool arrow,
            translation_unit::Context context);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    translation_unit::Context getContext() const override;
    Expression* getBase() const;
    const std::string& getMemberName() const;
    bool isArrow() const;

    void setMemberOffset(int offsetBytes);
    int getMemberOffset() const;

    // Base object's result symbol (struct value or pointer).
    void setBaseResultSymbol(semantic_analyzer::ValueEntry symbol);
    semantic_analyzer::ValueEntry* getBaseResultSymbol() const;

    // Temp holding address of the field (for stores through the member).
    void setFieldAddressSymbol(semantic_analyzer::ValueEntry symbol);
    semantic_analyzer::ValueEntry* getFieldAddressSymbol() const;

    semantic_analyzer::ValueEntry* getLvalueSymbol() const override;

private:
    std::unique_ptr<Expression> base;
    std::string memberName;
    bool arrow;
    translation_unit::Context context;
    int memberOffsetBytes { 0 };
    std::unique_ptr<semantic_analyzer::ValueEntry> baseResultSymbol;
    std::unique_ptr<semantic_analyzer::ValueEntry> fieldAddressSymbol;
};

} // namespace ast

#endif
