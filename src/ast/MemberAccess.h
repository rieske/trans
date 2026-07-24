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

    void setFieldAddressSymbol(semantic_analyzer::ValueEntry symbol);
    semantic_analyzer::ValueEntry* getFieldAddressSymbol() const;
    semantic_analyzer::ValueEntry* getLvalueSymbol() const override;

    // True when the base value is a pointer (arrow, or nested aggregate address).
    void setBaseIsPointer(bool value);
    bool baseIsPointer() const;

private:
    std::unique_ptr<Expression> base;
    std::string memberName;
    bool arrow;
    translation_unit::Context context;
    int memberOffsetBytes { 0 };
    bool baseIsPointerFlag { false };
    std::unique_ptr<semantic_analyzer::ValueEntry> fieldAddress;
};

} // namespace ast

#endif
