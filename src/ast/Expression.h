#ifndef _EXPR_NODE_H_
#define _EXPR_NODE_H_

#include <optional>

#include "AbstractSyntaxTreeNode.h"
#include "symbols/AnnotationStore.h"
#include "symbols/ValueEntry.h"
#include "types/IntegerConstant.h"

namespace ast {

class ParseEnvironment;

// Dual ownership of C type vs value:
//   expressionType() - C type for sizeof / isArray (on the node)
//   Result - ValueEntry only in AnnotationStore (ValueSlot::Result)
// ValueForm encodes dual-type cases without separate AST fields.
enum class ValueForm {
    Scalar,              // expressionType matches result type
    AggregateAddress,    // expressionType is the object; result holds its address (no load)
    // Function designator: expressionType is the function; Result is the address temp.
    FunctionDesignator,
};

class Expression: public AbstractSyntaxTreeNode {
public:
    virtual ~Expression() = default;

    virtual translation_unit::Context getContext() const = 0;

    // Parse-time typeof. Identifiers consult the environment; other productions
    // use their operands. nullopt means the operand has no type yet.
    virtual std::optional<type::Type> typeAtParseTime(const ParseEnvironment& environment) const = 0;

    void setType(const type::Type& type);
    // C type of the expression (sizeof / isArray / isStructure).
    type::Type expressionType() const;
    type::Type getType() const { return expressionType(); }
    bool hasExpressionType() const { return type.has_value(); }

    // Dual-type: array expressions keep the array as expression type.
    bool isArrayObjectType() const { return hasExpressionType() && expressionType().isArray(); }
    // Array expression type with pointer result (true dual ownership after SA).
    bool hasDecayedArrayValue(const symbols::AnnotationStore& store) const;

    // Type of the Result symbol after SA (prefer for arithmetic / assign source value).
    type::Type valueType(const symbols::AnnotationStore& store) const;

    virtual bool isLval() const;
    // Address temp for this expression (ValueSlot::Lvalue on the store).
    void setLvalueSymbol(symbols::AnnotationStore& store, symbols::ValueEntry address);
    symbols::ValueEntry* getLvalueSymbol(symbols::AnnotationStore& store) const;
    // Live object location for addressing: pointer Result, else pointer Lvalue, else Result.
    symbols::ValueEntry* addressSymbol(symbols::AnnotationStore& store) const;

    virtual bool evaluateConstant(type::IntegerConstant&) const { return false; }

    bool foldToHostLong(long& value) const {
        type::IntegerConstant c;
        if (!evaluateConstant(c)) {
            return false;
        }
        value = type::toHostLong(c);
        return true;
    }

    // Result lives only on the store (no node cache).
    void setTypeAndResult(symbols::AnnotationStore& store, symbols::ValueEntry resultSymbol);
    void setAggregateAddressResult(symbols::AnnotationStore& store, symbols::ValueEntry addressSymbol,
            const type::Type& aggregateType);
    void setFunctionDesignatorResult(symbols::AnnotationStore& store, symbols::ValueEntry addressSymbol,
            const type::Type& functionType);
    // Become src's value: C type, form, result, lvalue, address plan, value category.
    void takeValueFrom(Expression& src, symbols::AnnotationStore& store);

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
