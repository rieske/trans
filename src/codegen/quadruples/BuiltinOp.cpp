#include "BuiltinOp.h"

#include "codegen/AssemblyGenerator.h"

namespace codegen {

BuiltinOp::BuiltinOp(Kind kind, std::string operand, std::string result) :
        SingleOperandQuadruple { operand, result },
        kind { kind }
{
}

BuiltinOp::Kind BuiltinOp::getKind() const {
    return kind;
}

void BuiltinOp::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

void BuiltinOp::print(std::ostream& stream) const {
    const char* name = "builtin";
    switch (kind) {
    case Kind::Bswap16: name = "bswap16"; break;
    case Kind::Bswap32: name = "bswap32"; break;
    case Kind::Bswap64: name = "bswap64"; break;
    case Kind::Ctz: name = "ctz"; break;
    }
    stream << "\t" << getResult() << " := " << name << "(" << getOperand() << ")\n";
}

} // namespace codegen
