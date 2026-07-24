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
    void setBaseResultSymbol(symbols::ValueEntry symbol);
    symbols::ValueEntry* getBaseResultSymbol() const;

    // Temp holding address of the field (for stores through the member).
    void setFieldAddressSymbol(symbols::ValueEntry symbol);
    symbols::ValueEntry* getFieldAddressSymbol() const;

    symbols::ValueEntry* getLvalueSymbol() const override;

private:
    std::unique_ptr<Expression> base;
    std::string memberName;
    bool arrow;
    translation_unit::Context context;
    int memberOffsetBytes { 0 };
};

} // namespace ast

#endif
