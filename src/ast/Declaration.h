#ifndef _DECL_NODE_H_
#define _DECL_NODE_H_

#include <memory>
#include <string>

#include "../translation_unit/Context.h"
#include "AbstractSyntaxTreeNode.h"
#include "code_generator/ValueEntry.h"

namespace ast {

class Declaration: public AbstractSyntaxTreeNode {
public:
    virtual ~Declaration();

    std::string getName() const;
    int getDereferenceCount() const;

    translation_unit::Context getContext() const;

    void setHolder(code_generator::ValueEntry holder);
    code_generator::ValueEntry* getHolder() const;

    void setDereferenceCount(int dereferenceCount);

    static const std::string ID;

protected:
    Declaration(std::string name, const translation_unit::Context& context);

private:
    std::string name;
    int dereferenceCount { 0 };

    translation_unit::Context context;

    std::unique_ptr<code_generator::ValueEntry> holder { nullptr };
};

}

#endif // _DECL_NODE_H_
