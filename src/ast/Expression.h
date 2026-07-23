#ifndef _EXPR_NODE_H_
#define _EXPR_NODE_H_

#include <memory>
#include <optional>
#include <string>

#include "AbstractSyntaxTreeNode.h"
#include "semantic_analyzer/ValueEntry.h"

namespace ast {

class Expression: public AbstractSyntaxTreeNode {
public:
    virtual ~Expression() = default;

    virtual translation_unit::Context getContext() const = 0;

    void setType(const type::Type& type);
    type::Type getType() const;

    virtual bool isLval() const;
    virtual semantic_analyzer::ValueEntry* getLvalueSymbol() const;

    // Fold an integer constant expression; returns false if not a constant expression.
    virtual bool evaluateConstant(long& value) const { return false; }

    void setResultSymbol(semantic_analyzer::ValueEntry resultSymbol);
    bool hasResultSymbol() const;
    semantic_analyzer::ValueEntry* getResultSymbol() const;

    // Array-to-pointer decay (C 6.3.2.1): when set, codegen must emit
    // AddressOf(source, result) before using the (pointer-typed) result symbol.
    void setArrayDecaySource(std::string arraySymbolName);
    const std::string* getArrayDecaySource() const;

    // Implicit conversion target temp (float<->int formals / return). Result
    // remains the source type; codegen Assigns into this before the use site.
    void setResultConversionTarget(std::string targetSymbolName);
    const std::string* getResultConversionTarget() const;

protected:
    bool lval { false };

private:
    std::optional<type::Type> type;

    std::unique_ptr<semantic_analyzer::ValueEntry> resultSymbol { nullptr };
    std::optional<std::string> arrayDecaySource;
    std::optional<std::string> resultConversionTarget;
};

} // namespace ast

#endif // _EXPR_NODE_H_
