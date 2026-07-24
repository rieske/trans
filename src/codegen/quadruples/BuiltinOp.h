#ifndef BUILTINOP_H_
#define BUILTINOP_H_

#include "SingleOperandQuadruple.h"

namespace codegen {

// Lowering of expression-form GCC builtins that are not ordinary calls.
// Operand is evaluated once (single-eval contract for bswap of *p++).
class BuiltinOp: public SingleOperandQuadruple {
public:
    enum class Kind {
        Bswap16,
        Bswap32,
        Bswap64,
        Ctz
    };

    BuiltinOp(Kind kind, std::string operand, std::string result);
    virtual ~BuiltinOp() = default;

    Kind getKind() const;

    void generateCode(AssemblyGenerator& generator) const override;

private:
    void print(std::ostream& stream) const override;

    Kind kind;
};

} // namespace codegen

#endif // BUILTINOP_H_
