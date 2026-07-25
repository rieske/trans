#ifndef _EXPR_NODE_H_
#define _EXPR_NODE_H_

#include <memory>
#include <optional>
#include <string>

#include "AbstractSyntaxTreeNode.h"
#include "semantic_analyzer/ValueEntry.h"

namespace ast {

enum class ValueForm {
    Scalar,
    AggregateAddress,
};

class Expression: public AbstractSyntaxTreeNode {
public:
    virtual ~Expression() = default;

    virtual translation_unit::Context getContext() const = 0;

    void setType(const type::Type& type);
    type::Type expressionType() const;
    type::Type getType() const { return expressionType(); }
    bool hasExpressionType() const { return type.has_value(); }

    bool isArrayObjectType() const { return hasExpressionType() && expressionType().isArray(); }
    bool hasDecayedArrayValue() const;
    type::Type valueType() const;

    virtual bool isLval() const;
    virtual semantic_analyzer::ValueEntry* getLvalueSymbol() const;

    virtual bool evaluateConstant(long& value) const { return false; }

    void setTypeAndResult(semantic_analyzer::ValueEntry resultSymbol);
    void setAggregateAddressResult(semantic_analyzer::ValueEntry addressSymbol, const type::Type& aggregateType);

    void setResultSymbol(semantic_analyzer::ValueEntry resultSymbol) { setTypeAndResult(std::move(resultSymbol)); }

    bool hasResultSymbol() const;
    semantic_analyzer::ValueEntry* getResultSymbol() const;

    ValueForm valueForm() const { return form; }
    bool holdsAggregateAddress() const { return form == ValueForm::AggregateAddress; }

protected:
    bool lval { false };

private:
    std::optional<type::Type> type;
    std::unique_ptr<semantic_analyzer::ValueEntry> resultSymbol { nullptr };
    ValueForm form { ValueForm::Scalar };
};

} // namespace ast

#endif // _EXPR_NODE_H_
