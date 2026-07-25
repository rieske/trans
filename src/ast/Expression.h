#ifndef _EXPR_NODE_H_
#define _EXPR_NODE_H_

#include <memory>
#include <optional>
#include <string>

#include "AbstractSyntaxTreeNode.h"
#include "semantic_analyzer/ValueEntry.h"

namespace ast {

// How the result Value relates to expressionType() (finish-for-git dual ownership).
// Host uses AnnotationStore + setType/setResult; we keep Result on the node for now
// and encode dual ownership with ValueForm (same C rules).
enum class ValueForm {
    Scalar,              // expressionType matches result type
    AggregateAddress,    // expressionType is aggregate/array; result holds its address
    FunctionDesignator,  // decayed pointer-to-function temp; designatorName is the label
};

class Expression: public AbstractSyntaxTreeNode {
public:
    virtual ~Expression() = default;

    virtual translation_unit::Context getContext() const = 0;

    void setType(const type::Type& type);
    // C type of the expression (sizeof / isArray / isStructure). Host: expressionType().
    type::Type expressionType() const;
    type::Type getType() const { return expressionType(); }
    bool hasExpressionType() const { return type.has_value(); }

    // Dual-type: multi-dim rows / nested structs keep aggregate as expression type.
    bool isArrayObjectType() const { return hasExpressionType() && expressionType().isArray(); }
    // Array expression type with pointer result (true dual ownership after SA).
    bool hasDecayedArrayValue() const;

    // Type of the Result symbol after SA (prefer for arithmetic / assign source value).
    type::Type valueType() const;

    virtual bool isLval() const;
    virtual semantic_analyzer::ValueEntry* getLvalueSymbol() const;

    virtual bool evaluateConstant(long& value) const { return false; }

    // Scalar path: expression type and result both from the symbol.
    void setTypeAndResult(semantic_analyzer::ValueEntry resultSymbol);
    // Dual-type: expression type is aggregate/array; result holds its address.
    void setAggregateAddressResult(semantic_analyzer::ValueEntry addressSymbol, const type::Type& aggregateType);
    // Function designator decay: result is pointer-to-function temp; name is LEA label.
    void setFunctionDesignatorResult(semantic_analyzer::ValueEntry addressSymbol, std::string designatorName);

    void setResultSymbol(semantic_analyzer::ValueEntry resultSymbol) { setTypeAndResult(std::move(resultSymbol)); }

    bool hasResultSymbol() const;
    semantic_analyzer::ValueEntry* getResultSymbol() const;

    ValueForm valueForm() const { return form; }
    bool holdsAggregateAddress() const { return form == ValueForm::AggregateAddress; }
    bool holdsFunctionDesignator() const { return form == ValueForm::FunctionDesignator; }
    const std::string& functionDesignatorName() const;

protected:
    bool lval { false };

private:
    std::optional<type::Type> type;

    std::unique_ptr<semantic_analyzer::ValueEntry> resultSymbol { nullptr };
    ValueForm form { ValueForm::Scalar };
    std::string designatorName;
};

} // namespace ast

#endif // _EXPR_NODE_H_
