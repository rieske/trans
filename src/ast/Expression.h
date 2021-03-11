#ifndef _EXPR_NODE_H_
#define _EXPR_NODE_H_

#include <memory>
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

    void setResultSymbol(semantic_analyzer::ValueEntry resultSymbol);
    semantic_analyzer::ValueEntry* getResultSymbol() const;

protected:
    bool lval { false };

private:
    std::optional<type::Type> type;

    std::unique_ptr<semantic_analyzer::ValueEntry> resultSymbol { nullptr };
};

} // namespace ast

#endif // _EXPR_NODE_H_
