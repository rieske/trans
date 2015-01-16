#ifndef _DECL_NODE_H_
#define _DECL_NODE_H_

#include <memory>
#include <string>

#include "translation_unit/Context.h"
#include "code_generator/ValueEntry.h"
#include "Declarator.h"

namespace ast {

class DirectDeclarator: public Declarator {
public:
    virtual ~DirectDeclarator();

    std::string getName() const override;
    int getDereferenceCount() const override;

    translation_unit::Context getContext() const override;

    void setHolder(code_generator::ValueEntry holder) override;
    code_generator::ValueEntry* getHolder() const override;

    static const std::string ID;

protected:
    DirectDeclarator(std::string name, const translation_unit::Context& context);

private:
    std::string name;

    translation_unit::Context context;

    std::unique_ptr<code_generator::ValueEntry> holder { nullptr };
};

}

#endif // _DECL_NODE_H_
