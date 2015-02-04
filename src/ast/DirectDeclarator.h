#ifndef DIRECT_DECLARATOR_H_
#define DIRECT_DECLARATOR_H_

#include <memory>
#include <string>

#include "translation_unit/Context.h"
#include "code_generator/ValueEntry.h"
#include "AbstractSyntaxTreeNode.h"

namespace ast {

class DirectDeclarator: public AbstractSyntaxTreeNode {
public:
    virtual ~DirectDeclarator() = default;

    std::string getName() const;

    translation_unit::Context getContext() const;

    void setHolder(code_generator::ValueEntry holder);
    code_generator::ValueEntry* getHolder() const;

    static const std::string ID;

protected:
    DirectDeclarator(std::string name, const translation_unit::Context& context);

private:
    std::string name;

    translation_unit::Context context;

    std::unique_ptr<code_generator::ValueEntry> holder { nullptr };
};

}

#endif // DIRECT_DECLARATOR_H_
