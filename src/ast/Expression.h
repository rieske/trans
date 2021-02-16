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

    void setType(const FundamentalType& type);
    const FundamentalType& getType() const;

    virtual bool isLval() const;
    virtual semantic_analyzer::ValueEntry* getLvalueSymbol() const;

    static const std::string ID;

    void setResultSymbol(semantic_analyzer::ValueEntry resultSymbol);
    semantic_analyzer::ValueEntry* getResultSymbol() const;

protected:
    bool lval { false };

private:
    std::unique_ptr<FundamentalType> type;

    std::unique_ptr<semantic_analyzer::ValueEntry> resultSymbol { nullptr };
};

} // namespace ast

#endif // _EXPR_NODE_H_
