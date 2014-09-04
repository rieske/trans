#ifndef _DECL_NODE_H_
#define _DECL_NODE_H_

#include <string>

#include "../scanner/TranslationUnitContext.h"
#include "AbstractSyntaxTreeNode.h"

class SymbolEntry;

namespace ast {

class DirectDeclaration;
class Pointer;

class Declaration: public AbstractSyntaxTreeNode {
public:
    virtual ~Declaration();

    std::string getName() const;
    std::string getType() const;

    const TranslationUnitContext& getContext() const;

    void setHolder(SymbolEntry* holder);
    SymbolEntry* getHolder() const;

    void dereference(std::string pointerType);

    static const std::string ID;

protected:
    Declaration(std::string name, std::string type, const TranslationUnitContext& context);

    std::string name;
    std::string type;

private:
    TranslationUnitContext context;

    SymbolEntry *holder;
};

}

#endif // _DECL_NODE_H_
