#include "Expression.h"

#include <cassert>
#include <stdexcept>

#include "../semantic_analyzer/ValueEntry.h"
#include "types/FundamentalType.h"

namespace ast {

const std::string Expression::ID { "<exp>" };

void Expression::setType(const FundamentalType& type) {
    this->type = std::unique_ptr<FundamentalType> { type.clone() };
}

const FundamentalType& Expression::getType() const {
    if (!type) {
        throw std::runtime_error { "type is null" };
    }
    return *type;
}

void Expression::setResultSymbol(semantic_analyzer::ValueEntry resultSymbol) {
    this->resultSymbol = std::make_unique<semantic_analyzer::ValueEntry>(resultSymbol);
    setType(this->resultSymbol->getType());
}

semantic_analyzer::ValueEntry* Expression::getResultSymbol() const {
    assert(resultSymbol);
    return resultSymbol.get();
}

bool Expression::isLval() const {
    return lval;
}

}

