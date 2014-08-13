#ifndef _DECL_NODE_H_
#define _DECL_NODE_H_

#include <stddef.h>
#include <string>

#include "AbstractSyntaxTreeNode.h"

#include "../code_generator/symbol_entry.h"

namespace semantic_analyzer {

class DirectDeclaration;
class Pointer;

class Declaration: public AbstractSyntaxTreeNode {
public:
    virtual ~Declaration();

    std::string getName() const;
    std::string getType() const;
    size_t getLineNumber() const;

    void setHolder(SymbolEntry* holder);
    SymbolEntry* getHolder() const;

    void dereference(std::string pointerType);

    static const std::string ID;

protected:
    Declaration(std::string name, std::string type);

    std::string name;
    std::string type;
    size_t lineNumber;

private:
    SymbolEntry *holder;
};

}

#endif // _DECL_NODE_H_
