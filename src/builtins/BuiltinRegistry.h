#ifndef BUILTINS_BUILTIN_REGISTRY_H_
#define BUILTINS_BUILTIN_REGISTRY_H_

#include <functional>
#include <string>
#include <variant>

#include "symbols/BuiltinPlan.h"
#include "types/Type.h"

namespace builtins {

struct SimpleBuiltin {
    symbols::BuiltinPlan plan;
};
struct TypeNameReturn {};
using BuiltinKind = std::variant<SimpleBuiltin, TypeNameReturn>;

// Product-name / arity / return-type table for expression-form GCC builtins.
// Real calls use CallPlan (Direct|Indirect). Kind is a closed sum type:
// Simple carries a BuiltinPlan; TypeNameReturn (va_arg) waits for SA.
// offsetof is a parse-time ConstantExpression from GnuExtensions.

struct BuiltinDescriptor {
    BuiltinKind kind { SimpleBuiltin { symbols::VaEndPlan {} } };
    int minArity { 1 };
    int maxArity { 1 };
    type::Type returnType { type::signedInteger() };
    type::Type argType { type::unsignedLong() };
};

std::optional<BuiltinDescriptor> lookupBuiltin(const std::string& designatorName);

// Names that get a FunctionEntry for designator / & uses (bswap, ctz, alloca).
void forEachDesignatorBuiltin(
        const std::function<void(const char* name, const BuiltinDescriptor& desc)>& fn);

inline bool builtinArityOk(const BuiltinDescriptor& d, std::size_t argc) {
    return static_cast<int>(argc) >= d.minArity && static_cast<int>(argc) <= d.maxArity;
}

} // namespace builtins

#endif // BUILTINS_BUILTIN_REGISTRY_H_
