#ifndef _EXPR_NODE_H_
#define _EXPR_NODE_H_

#include <memory>
#include <string>

#include "AbstractSyntaxTreeNode.h"
#include "TypeInfo.h"

class TranslationUnitContext;

class SymbolEntry;

namespace ast {

class Expression: public AbstractSyntaxTreeNode {
public:
    virtual ~Expression() {
    }

    virtual const TranslationUnitContext& getContext() const = 0;

    void setTypeInfo(TypeInfo typeInfo);
    TypeInfo getTypeInfo() const;

    bool isLval() const;

    static const std::string ID;

    void setResultHolder(SymbolEntry* resultHolder);
    SymbolEntry* getResultHolder() const;

protected:
    bool lval { false };

private:
    std::unique_ptr<TypeInfo> typeInfo;

    SymbolEntry* resultHolder { nullptr };
};

}

#endif // _EXPR_NODE_H_
