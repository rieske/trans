#ifndef _EXPR_NODE_H_
#define _EXPR_NODE_H_

#include <memory>
#include <string>

#include "AbstractSyntaxTreeNode.h"
#include "BasicType.h"
#include "TypeInfo.h"

class SymbolEntry;

namespace semantic_analyzer {

class Expression: public AbstractSyntaxTreeNode {
public:
    virtual ~Expression() {
    }

    void setTypeInfo(TypeInfo typeInfo);
    TypeInfo getTypeInfo() const;

    BasicType getBasicType() const;
    std::string getExtendedType() const;

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
