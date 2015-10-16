#ifndef _TERM_NODE_H_
#define _TERM_NODE_H_

#include <string>

#include "translation_unit/Context.h"
#include "Expression.h"

namespace ast {

class IdentifierExpression: public Expression {
public:
    IdentifierExpression(std::string identifier, translation_unit::Context context);
    virtual ~IdentifierExpression();

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    translation_unit::Context getContext() const override;
    std::string getIdentifier() const;

private:
    std::string identifier;
    translation_unit::Context context;
};

}

#endif // _TERM_NODE_H_
