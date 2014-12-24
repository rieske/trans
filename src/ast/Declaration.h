#ifndef _DECL_NODE_H_
#define _DECL_NODE_H_

#include <string>

#include "../scanner/TranslationUnitContext.h"
#include "../code_generator/ValueEntry.h"

#include "AbstractSyntaxTreeNode.h"

namespace ast {

class Declaration: public AbstractSyntaxTreeNode {
public:
    virtual ~Declaration();

    std::string getName() const;
    int getDereferenceCount() const;

    const TranslationUnitContext& getContext() const;

    void setHolder(code_generator::ValueEntry holder);
    code_generator::ValueEntry* getHolder() const;

    void setDereferenceCount(int dereferenceCount);

    static const std::string ID;

protected:
    Declaration(std::string name, const TranslationUnitContext& context);

private:
    std::string name;
    int dereferenceCount { 0 };

    TranslationUnitContext context;

    std::unique_ptr<code_generator::ValueEntry> holder { nullptr };
};

}

#endif // _DECL_NODE_H_
