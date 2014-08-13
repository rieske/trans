#ifndef _EXPR_NODE_H_
#define _EXPR_NODE_H_

#include <string>

#include "AbstractSyntaxTreeNode.h"
#include "BasicType.h"

#include "../code_generator/symbol_entry.h"

namespace semantic_analyzer {

class Expression: public AbstractSyntaxTreeNode {
public:
    virtual ~Expression() {
    }

    virtual BasicType getBasicType() const;
    virtual std::string getExtendedType() const;
    virtual std::string getValue() const;

    virtual SymbolEntry *getLval() const;

    static const std::string ID;

    void setResultHolder(SymbolEntry* resultHolder);
    SymbolEntry* getResultHolder() const;
protected:
    void saveExpressionAttributes(const Expression& expression);

    BasicType basicType;
    std::string extended_type;
    std::string value;

    SymbolEntry *lval { nullptr }; // FIXME: make me private

private:
    SymbolEntry* resultHolder { nullptr };
};

}

#endif // _EXPR_NODE_H_
