#ifndef AST_BUILTIN_REGISTRY_H_
#define AST_BUILTIN_REGISTRY_H_

#include <optional>
#include <string>

#include "FunctionCall.h"
#include "types/Type.h"

namespace ast {

// How codegen lowers an expression-form builtin (table-driven; no kind switch forests).
enum class BuiltinLowering {
    ConstantZero,   // __builtin_constant_p -> product_approx 0
    CallAs,         // emit call to syntheticName (alloca -> malloc)
    BuiltinOp,      // bswap / ctz quadruple
    VaStart,
    VaEnd,
    VaCopy,
    VaArg
};

// Table-driven description of expression-form GCC builtins.
struct BuiltinDescriptor {
    FunctionCall::BuiltinKind kind { FunctionCall::BuiltinKind::None };
    BuiltinLowering lowering { BuiltinLowering::ConstantZero };
    int minArity { 1 };
    int maxArity { 1 };
    type::Type returnType { type::signedInteger() };
    // Synthetic FunctionEntry arg type (single formal for symbol table).
    type::Type syntheticArgType { type::unsignedLong() };
    // CallAs target or display name; null uses designator / default.
    const char* syntheticName { nullptr };
    // For BuiltinOp lowering: which op (maps 1:1 to codegen::BuiltinOp::Kind in visitor).
    // Reuses FunctionCall::BuiltinKind Bswap16/32/64/Ctz — no parallel int encoding.
};

// Map a designator name (or already-set BuiltinKind) to a descriptor.
std::optional<BuiltinDescriptor> lookupBuiltin(
        FunctionCall::BuiltinKind existingKind,
        const std::string& designatorName,
        const type::Type* vaArgResultType);

FunctionCall::BuiltinKind builtinKindForName(const std::string& name);

// True when argc is within [minArity, maxArity].
inline bool builtinArityOk(const BuiltinDescriptor& d, std::size_t argc) {
    return static_cast<int>(argc) >= d.minArity && static_cast<int>(argc) <= d.maxArity;
}

} // namespace ast

#endif // AST_BUILTIN_REGISTRY_H_
