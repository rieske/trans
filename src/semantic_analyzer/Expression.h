#ifndef _EXPR_NODE_H_
#define _EXPR_NODE_H_

#include <string>
#include <vector>

#include "../code_generator/quadruple.h"
#include "BasicType.h"
#include "NonterminalNode.h"

class SymbolTable;

namespace semantic_analyzer {

class Expression: public NonterminalNode {
public:
    virtual ~Expression() {
    }

    virtual BasicType getBasicType() const;
    virtual string getExtendedType() const;
    virtual string getValue() const;

    virtual SymbolEntry *getPlace() const;
    virtual SymbolEntry *getLval() const;

    vector<Quadruple *> getBackpatchList() const;

    static const std::string ID;

    void backpatch();

    void setResultHolder(SymbolEntry* resultHolder);
    SymbolEntry* getResultHolder() const;
protected:
    Expression(SymbolTable *st, unsigned ln);

    void saveExpressionAttributes(const Expression& expression);

    BasicType basicType;
    string extended_type;
    string value;

    SymbolEntry *resultPlace { nullptr };
    SymbolEntry *lval { nullptr };

    vector<Quadruple *> backpatchList;

private:
    SymbolEntry* resultHolder { nullptr };
};

}

#endif // _EXPR_NODE_H_
