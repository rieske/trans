#include "FunctionSpecifier.h"

namespace ast {

FunctionSpecifier FunctionSpecifier::INLINE(translation_unit::Context context) {
    return { FunctionSpec::INLINE, context };
}

FunctionSpecifier FunctionSpecifier::NORETURN(translation_unit::Context context) {
    return { FunctionSpec::NORETURN, context };
}

FunctionSpecifier::FunctionSpecifier(FunctionSpec spec, translation_unit::Context context) :
        spec { spec },
        context { context }
{
}

FunctionSpec FunctionSpecifier::getSpec() const {
    return spec;
}

translation_unit::Context FunctionSpecifier::getContext() const {
    return context;
}

} // namespace ast
