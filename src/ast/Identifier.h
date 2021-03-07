#ifndef IDENTIFIER_H_
#define IDENTIFIER_H_

#include <memory>
#include <vector>

#include "ast/DirectDeclarator.h"
#include "ast/Pointer.h"
#include "ast/TerminalSymbol.h"

namespace ast {

class Identifier: public DirectDeclarator {
public:
    Identifier(TerminalSymbol identifier);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    type::Type getFundamentalType(std::vector<Pointer> indirection, const type::Type& baseType) override;

};

} // namespace ast

#endif // IDENTIFIER_H_
