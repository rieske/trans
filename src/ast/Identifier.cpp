#include "Identifier.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"
#include "../types/PointerType.h"
#include "TerminalSymbol.h"

namespace ast {

Identifier::Identifier(TerminalSymbol identifier) :
        DirectDeclarator(identifier.value, identifier.context)
{
}

void Identifier::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

std::unique_ptr<FundamentalType> Identifier::getFundamentalType(std::vector<Pointer> indirection, const FundamentalType& baseType) {
    std::unique_ptr<FundamentalType> type = std::unique_ptr<FundamentalType> { baseType.clone() };
    for (Pointer pointer : indirection) {
        std::unique_ptr<FundamentalType> pointerType = std::make_unique<PointerType>(std::move(type), pointer.getQualifiers());
        type = std::move(pointerType);
    }
    return type;
}

} /* namespace semantic_analyzer */
