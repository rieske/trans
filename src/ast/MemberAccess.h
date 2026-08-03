#ifndef MEMBERACCESS_H_
#define MEMBERACCESS_H_

#include <memory>
#include <string>

#include "Expression.h"
#include "symbols/AnnotationStore.h"
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

    // Field offset / AddressBaseResolved: AddressPlan FieldPlan on the store.
    // Address temp: ValueSlot::Lvalue (default lvalueAnnotation).

private:
    std::unique_ptr<Expression> base;
    std::string memberName;
    bool arrow;
    translation_unit::Context context;
};

} // namespace ast

#endif
