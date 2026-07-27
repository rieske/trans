#ifndef _EXPR_NODE_H_
#define _EXPR_NODE_H_

#include <optional>
#include <string>

#include "AbstractSyntaxTreeNode.h"
#include "symbols/AnnotationStore.h"
#include "symbols/ValueEntry.h"

namespace ast {

// Dual ownership of C type vs value (finish-for-git protocol):
//   expressionType() — C type for sizeof / isArray (on the node)
//   Result — ValueEntry only in AnnotationStore (ValueSlot::Result)
// ValueForm encodes dual-type cases without separate AST fields.
enum class ValueForm {
    Scalar,              // expressionType matches result type
    AggregateAddress,    // expressionType is aggregate/array; result holds its address
    // Decayed pointer-to-function temp; LEA label lives on FunctionDesignatorPlan (store).
    FunctionDesignator,
};

class Expression: public AbstractSyntaxTreeNode {
public:
    virtual ~Expression() = default;

    virtual translation_unit::Context getContext() const = 0;

    void setType(const type::Type& type);
    // C type of the expression (sizeof / isArray / isStructure).
    type::Type expressionType() const;
    type::Type getType() const { return expressionType(); }
    bool hasExpressionType() const { return type.has_value(); }

    // Dual-type: multi-dim rows / nested structs keep aggregate as expression type.
    bool isArrayObjectType() const { return hasExpressionType() && expressionType().isArray(); }
    // Array expression type with pointer result (true dual ownership after SA).
    bool hasDecayedArrayValue(const symbols::AnnotationStore& store) const;

    // Type of the Result symbol after SA (prefer for arithmetic / assign source value).
    type::Type valueType(const symbols::AnnotationStore& store) const;

    virtual bool isLval() const;
    // Address temp for this expression (ValueSlot::Lvalue on the store).
    void setLvalueSymbol(symbols::AnnotationStore& store, symbols::ValueEntry address);
    virtual symbols::ValueEntry* getLvalueSymbol(symbols::AnnotationStore& store) const;

    virtual bool evaluateConstant(long& value) const { return false; }

    // Result lives only on the store (no node cache).
    void setTypeAndResult(symbols::AnnotationStore& store, symbols::ValueEntry resultSymbol);
    void setAggregateAddressResult(symbols::AnnotationStore& store, symbols::ValueEntry addressSymbol,
            const type::Type& aggregateType);
    void setFunctionDesignatorResult(symbols::AnnotationStore& store, symbols::ValueEntry addressSymbol);

    void setResultSymbol(symbols::AnnotationStore& store, symbols::ValueEntry resultSymbol) {
        setTypeAndResult(store, std::move(resultSymbol));
    }

    bool hasResultSymbol(const symbols::AnnotationStore& store) const;
    // Required Result after successful SA — asserts if missing (same contract as AnnotationStore::result).
    // Probe with hasResultSymbol before calling when the expression may have failed analysis.
    symbols::ValueEntry* getResultSymbol(symbols::AnnotationStore& store) const;

    ValueForm valueForm() const { return form; }
    bool holdsAggregateAddress() const { return form == ValueForm::AggregateAddress; }
    bool holdsFunctionDesignator() const { return form == ValueForm::FunctionDesignator; }

protected:
    bool lval { false };

private:
    std::optional<type::Type> type;
    ValueForm form { ValueForm::Scalar };
};

} // namespace ast

#endif // _EXPR_NODE_H_
