#ifndef IDENTIFIER_H_
#define IDENTIFIER_H_

#include "DirectDeclarator.h"

namespace ast {

class TerminalSymbol;

class Identifier: public DirectDeclarator {
public:
    Identifier(TerminalSymbol identifier);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
};

} /* namespace ast */

#endif /* IDENTIFIER_H_ */
