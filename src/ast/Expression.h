#ifndef _EXPR_NODE_H_
#define _EXPR_NODE_H_

#include <optional>
#include <string>

#include "AbstractSyntaxTreeNode.h"
#include "symbols/ValueEntry.h"
#include "types/Type.h"

// C expression type lives on the node (syntax/type structure).
// Value annotations live only in symbols::AnnotationStore (side table).
//
// Dual ownership (do not collapse):
//   expressionType()  — C type for sizeof / isArray / isFunction (syntax)
//   valueType(store)  — Result-symbol type after SA (requires Result; fail-closed)
// ValueForm encodes dual-type cases without soft setType+setResult order.
// Function designator name lives only on FunctionDesignatorPlan (store).

namespace symbols {
class AnnotationStore;
}

namespace ast {

enum class ValueForm {
    Scalar,              // expressionType matches result type
    AggregateAddress,    // expressionType is aggregate/array; result holds its address
    FunctionDesignator,  // decayed pointer-to-function temp; label on FunctionDesignatorPlan
};

class Expression: public AbstractSyntaxTreeNode {
public:
    virtual ~Expression() = default;

    virtual translation_unit::Context getContext() const = 0;

    void setType(const type::Type& type);
    type::Type expressionType() const;
    bool hasExpressionType() const { return type.has_value(); }

    bool isArrayObjectType() const { return hasExpressionType() && expressionType().isArray(); }

    // Derived dual-type probe (tests). Production CG uses holdsAggregateAddress().
    bool hasDecayedArrayValue(const symbols::AnnotationStore& store) const;

    type::Type valueType(const symbols::AnnotationStore& store) const;

    virtual bool isLval() const;

    // Polymorphic lvalue address for assignment / & / decay reuse.
    // Default: ValueSlot::Lvalue; compound literals may use Object.
    virtual symbols::ValueEntry* lvalueAnnotation(symbols::AnnotationStore& store) const;

    virtual bool evaluateConstant(long& value) const { return false; }

    // Dual-type write protocol (expression type + Result slot).
    void setTypeAndResult(symbols::AnnotationStore& store, symbols::ValueEntry resultSymbol);
    void setAggregateAddressResult(symbols::AnnotationStore& store, symbols::ValueEntry addressSymbol,
            const type::Type& aggregateType);
    // Function designator: Result is pointer temp; label name goes on FunctionDesignatorPlan.
    void setFunctionDesignatorResult(symbols::AnnotationStore& store, symbols::ValueEntry addressSymbol,
            std::string functionName);

    bool hasResult(const symbols::AnnotationStore& store) const;
    symbols::ValueEntry* result(symbols::AnnotationStore& store) const;

    ValueForm valueForm() const { return form; }
    bool holdsAggregateAddress() const { return form == ValueForm::AggregateAddress; }
    bool holdsFunctionDesignator() const { return form == ValueForm::FunctionDesignator; }

    // Designator label from FunctionDesignatorPlan (requires holdsFunctionDesignator).
    const std::string* functionDesignatorName(const symbols::AnnotationStore& store) const;

protected:
    bool lval { false };

private:
    std::optional<type::Type> type;
    ValueForm form { ValueForm::Scalar };
};

} // namespace ast

#endif // _EXPR_NODE_H_
