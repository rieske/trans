#ifndef FUNCTIONSPECIFIER_H_
#define FUNCTIONSPECIFIER_H_

namespace ast {

enum class FunctionSpec {
    INLINE, NORETURN
};

class FunctionSpecifier {
public:
    static FunctionSpecifier INLINE();
    static FunctionSpecifier NORETURN();

    FunctionSpec getSpec() const;

private:
    explicit FunctionSpecifier(FunctionSpec spec);

    FunctionSpec spec;
};

} // namespace ast

#endif // FUNCTIONSPECIFIER_H_
