#ifndef SEMANTIC_BUILTINANALYSIS_H_
#define SEMANTIC_BUILTINANALYSIS_H_

#include <optional>
#include <string>

#include "ast/FunctionCall.h"
#include "types/Type.h"

namespace semantic_analyzer {

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
    ast::FunctionCall::BuiltinKind kind { ast::FunctionCall::BuiltinKind::None };
    BuiltinLowering lowering { BuiltinLowering::ConstantZero };
    int minArity { 1 };
    int maxArity { 1 };
    type::Type returnType { type::signedInteger() };
    // Synthetic FunctionEntry arg type (single formal for symbol table).
    type::Type syntheticArgType { type::unsignedLong() };
    // CallAs target or display name; null uses designator / default.
    const char* syntheticName { nullptr };
    // BuiltinOp kind index: 0=Bswap16,1=Bswap32,2=Bswap64,3=Ctz (matches BuiltinOp::Kind).
    int builtinOpKind { 0 };
};

// Map a designator name (or already-set BuiltinKind) to a descriptor.
std::optional<BuiltinDescriptor> lookupBuiltin(
        ast::FunctionCall::BuiltinKind existingKind,
        const std::string& designatorName,
        const type::Type* vaArgResultType);

ast::FunctionCall::BuiltinKind builtinKindForName(const std::string& name);

// True when argc is within [minArity, maxArity].
inline bool builtinArityOk(const BuiltinDescriptor& d, std::size_t argc) {
    return static_cast<int>(argc) >= d.minArity && static_cast<int>(argc) <= d.maxArity;
}

} // namespace semantic_analyzer

#endif // SEMANTIC_BUILTINANALYSIS_H_
