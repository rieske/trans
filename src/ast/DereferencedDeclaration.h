#ifndef DEREFERENCEDDECLARATION_H_
#define DEREFERENCEDDECLARATION_H_

#include <memory>
#include <string>

#include "Declaration.h"

namespace ast {

class DereferencedDeclaration: public Declaration {
public:
    DereferencedDeclaration(std::unique_ptr<Declaration> declaration);
    virtual ~DereferencedDeclaration();

    virtual void accept(AbstractSyntaxTreeVisitor& visitor) override;

    virtual std::string getName() const override;
    virtual int getDereferenceCount() const override;

    virtual translation_unit::Context getContext() const override;

    virtual void setHolder(code_generator::ValueEntry holder) override;
    virtual code_generator::ValueEntry* getHolder() const override;

private:
    std::unique_ptr<Declaration> declaration;
};

} /* namespace ast */

#endif /* DEREFERENCEDDECLARATION_H_ */
