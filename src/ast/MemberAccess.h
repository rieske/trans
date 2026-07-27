#ifndef MEMBERACCESS_H_
#define MEMBERACCESS_H_

#include <memory>
#include <string>

#include "Expression.h"
#include "translation_unit/Context.h"

namespace ast {

// postfix . id  or  postfix -> id
// Field offset / AddressBaseMode live in symbols::AddressPlan (Field) on the store.
class MemberAccess: public Expression {
public:
    MemberAccess(std::unique_ptr<Expression> base, std::string memberName, bool arrow,
            translation_unit::Context context);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    translation_unit::Context getContext() const override;
    Expression* getBase() const;
    const std::string& getMemberName() const;
    bool isArrow() const;

    void setFieldAddressSymbol(symbols::AnnotationStore& store, symbols::ValueEntry symbol);
    symbols::ValueEntry* getFieldAddressSymbol(symbols::AnnotationStore& store) const;
    symbols::ValueEntry* getLvalueSymbol(symbols::AnnotationStore& store) const override;

private:
    std::unique_ptr<Expression> base;
    std::string memberName;
    bool arrow;
    translation_unit::Context context;
};

} // namespace ast

#endif
