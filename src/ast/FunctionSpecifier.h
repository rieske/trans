#ifndef FUNCTIONSPECIFIER_H_
#define FUNCTIONSPECIFIER_H_

#include "translation_unit/Context.h"

namespace ast {

enum class FunctionSpec {
    INLINE, NORETURN
};

class FunctionSpecifier {
public:
    static FunctionSpecifier INLINE(translation_unit::Context context);
    static FunctionSpecifier NORETURN(translation_unit::Context context);

    FunctionSpec getSpec() const;
    translation_unit::Context getContext() const;

private:
    FunctionSpecifier(FunctionSpec spec, translation_unit::Context context);

    FunctionSpec spec;
    translation_unit::Context context;
};

} // namespace ast

#endif // FUNCTIONSPECIFIER_H_
