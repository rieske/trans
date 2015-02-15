#ifndef IDENTIFIER_H_
#define IDENTIFIER_H_

#include <memory>
#include <vector>

#include "DirectDeclarator.h"
#include "Pointer.h"

namespace ast {
class StoredType;
} /* namespace ast */

namespace ast {

class TerminalSymbol;

class Identifier: public DirectDeclarator {
public:
    Identifier(TerminalSymbol identifier);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    std::unique_ptr<FundamentalType> getFundamentalType(std::vector<Pointer> indirection, const FundamentalType& baseType) override;

};

} /* namespace ast */

#endif /* IDENTIFIER_H_ */
