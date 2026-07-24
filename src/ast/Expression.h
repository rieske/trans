#ifndef _EXPR_NODE_H_
#define _EXPR_NODE_H_

#include <memory>
#include <optional>
#include <string>

#include "AbstractSyntaxTreeNode.h"
#include "symbols/ValueEntry.h"
#include "types/Type.h"

// C expression type lives on the node (syntax/type structure).
// Value annotations (result symbol, decay, conversion) live in symbols::AnnotationStore
// keyed by node address — not as members on the AST object.

namespace ast {

class Expression: public AbstractSyntaxTreeNode {
public:
    virtual ~Expression() = default;

    virtual translation_unit::Context getContext() const = 0;

    // C expression type (sizeof, isArray). Fail-closed once read.
    void setType(const type::Type& type);
    type::Type expressionType() const;
    bool hasExpressionType() const { return type.has_value(); }

    // Result-symbol type after SA (decay temps, etc.). Prefer for arithmetic /
    // codegen value paths. Falls back to expressionType() if no result symbol.
    type::Type valueType() const;

    virtual bool isLval() const;
    virtual symbols::ValueEntry* getLvalueSymbol() const;

    virtual bool evaluateConstant(long& value) const { return false; }

    // Single-type: set expression type from the symbol and store the value annotation.
    void setTypedResult(symbols::ValueEntry resultSymbol);
    // Dual ownership: expression type already set; store value annotation only.
    void setResultSymbol(symbols::ValueEntry resultSymbol);
    bool hasResultSymbol() const;
    symbols::ValueEntry* getResultSymbol() const;

    void setArrayDecaySource(std::string arraySymbolName);
    const std::string* getArrayDecaySource() const;

    void setResultConversionTarget(std::string targetSymbolName);
    const std::string* getResultConversionTarget() const;

protected:
    bool lval { false };

private:
    std::optional<type::Type> type;
};

} // namespace ast

#endif // _EXPR_NODE_H_
