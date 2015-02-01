#ifndef DEREFERENCEDDECLARATION_H_
#define DEREFERENCEDDECLARATION_H_

#include <memory>
#include <string>

#include "Declarator.h"

namespace ast {

class DereferencedDeclarator: public Declarator {
public:
    DereferencedDeclarator(std::unique_ptr<Declarator> declaration);
    virtual ~DereferencedDeclarator();

    virtual void accept(AbstractSyntaxTreeVisitor& visitor) override;

    virtual std::string getName() const override;
    virtual int getDereferenceCount() const override;

    virtual translation_unit::Context getContext() const override;

    virtual void setHolder(code_generator::ValueEntry holder) override;
    virtual code_generator::ValueEntry* getHolder() const override;

private:
    std::unique_ptr<Declarator> declaration;
};

} /* namespace ast */

#endif /* DEREFERENCEDDECLARATION_H_ */
