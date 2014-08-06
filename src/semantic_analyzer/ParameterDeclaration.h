#ifndef _PARAM_DECL_NODE_H_
#define _PARAM_DECL_NODE_H_

#include <memory>
#include <string>

#include "BasicType.h"
#include "TypeSpecifier.h"

class SymbolEntry;
class SymbolTable;

namespace semantic_analyzer {

class Declaration;
class TerminalSymbol;

class ParameterDeclaration: public NonterminalNode {
public:
    ParameterDeclaration(TypeSpecifier typeSpecifier, std::unique_ptr<Declaration> declaration, SymbolTable *st);
    virtual ~ParameterDeclaration();

    BasicType getBasicType() const;
    string getExtendedType() const;

    SymbolEntry *getPlace() const;

    static const std::string ID;

    void accept(AbstractSyntaxTreeVisitor& visitor) const override;

    const TypeSpecifier type;
    const std::unique_ptr<Declaration> declaration;

private:
    string name;
    BasicType basicType;
    string extended_type;

    SymbolEntry *place;
};

}

#endif // _PARAM_DECL_NODE_H_
