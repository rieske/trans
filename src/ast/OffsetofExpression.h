#ifndef OFFSETEXPRESSION_H_
#define OFFSETEXPRESSION_H_

#include <optional>
#include <string>

#include "Expression.h"
#include "TypeName.h"
#include "translation_unit/Context.h"

namespace ast {

// __builtin_offsetof(type_name, member): not a FunctionCall. SA folds the
// record offset; evaluateConstant serves ICE (array bounds / BUILD_ASSERT).
class OffsetofExpression: public Expression {
public:
    OffsetofExpression(TypeName typeName, std::string memberName,
            translation_unit::Context context);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    translation_unit::Context getContext() const override;

    TypeName& getTypeName();
    const TypeName& getTypeName() const;
    const std::string& getMemberName() const;

    void setFoldedInteger(long value);
    bool evaluateConstant(long& value) const override;

private:
    TypeName typeName;
    std::string memberName;
    translation_unit::Context context;
    std::optional<long> foldedInteger_;
};

} // namespace ast

#endif
