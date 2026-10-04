#include "FunctionSpecifier.h"

namespace ast {

FunctionSpecifier FunctionSpecifier::INLINE() {
    return FunctionSpecifier { FunctionSpec::INLINE };
}

FunctionSpecifier FunctionSpecifier::NORETURN() {
    return FunctionSpecifier { FunctionSpec::NORETURN };
}

FunctionSpecifier::FunctionSpecifier(FunctionSpec spec) :
        spec { spec } {
}

FunctionSpec FunctionSpecifier::getSpec() const {
    return spec;
}

} // namespace ast
