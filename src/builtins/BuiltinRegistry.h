#ifndef BUILTINS_BUILTIN_REGISTRY_H_
#define BUILTINS_BUILTIN_REGISTRY_H_

#include <string>
#include <variant>

#include "symbols/BuiltinPlan.h"
#include "types/Type.h"

namespace builtins {

struct SimpleBuiltin {
    symbols::BuiltinPlan plan;
};
struct TypeNameReturn {};
struct SyntheticCall {
    std::string callee;
};
using BuiltinKind = std::variant<SimpleBuiltin, TypeNameReturn, SyntheticCall>;

// Product-name / arity / return-type table for expression-form GCC builtins.
// Real calls use CallPlan (Direct|Indirect). Kind is a closed sum type:
// Simple carries a BuiltinPlan; TypeNameReturn (va_arg) waits for SA;
// SyntheticCall is alloca -> malloc. offsetof is OffsetofExpression, not here.

struct BuiltinDescriptor {
    BuiltinKind kind { SimpleBuiltin { symbols::ConstantZeroPlan {} } };
    int minArity { 1 };
    int maxArity { 1 };
    type::Type returnType { type::signedInteger() };
    type::Type syntheticArgType { type::unsignedLong() };
};

std::optional<BuiltinDescriptor> lookupBuiltin(const std::string& designatorName);

inline bool builtinArityOk(const BuiltinDescriptor& d, std::size_t argc) {
    return static_cast<int>(argc) >= d.minArity && static_cast<int>(argc) <= d.maxArity;
}

} // namespace builtins

#endif // BUILTINS_BUILTIN_REGISTRY_H_
