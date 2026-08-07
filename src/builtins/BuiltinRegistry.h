#ifndef BUILTINS_BUILTIN_REGISTRY_H_
#define BUILTINS_BUILTIN_REGISTRY_H_

#include <optional>
#include <string>

#include "symbols/BuiltinPlan.h"
#include "types/Type.h"

namespace builtins {

// Product-name / arity / return-type table for expression-form GCC builtins.
// Real calls use CallPlan (Direct|Indirect). Product lowers via BuiltinPlan;
// __builtin_alloca rewrites to a Direct call of syntheticCallee ("malloc").

struct BuiltinDescriptor {
    // If set, SA writes BuiltinPlan (not CallPlan).
    std::optional<symbols::BuiltinPlan> builtinPlan;
    // If set, SA writes CallPlan::Direct + FunctionEntry{syntheticCallee} (e.g. alloca → malloc).
    std::optional<std::string> syntheticCallee;
    int minArity { 1 };
    int maxArity { 1 };
    type::Type returnType { type::signedInteger() };
    type::Type syntheticArgType { type::unsignedLong() };
};

std::optional<BuiltinDescriptor> lookupBuiltin(
        const std::string& designatorName,
        const type::Type* vaArgResultType);

inline bool builtinArityOk(const BuiltinDescriptor& d, std::size_t argc) {
    return static_cast<int>(argc) >= d.minArity && static_cast<int>(argc) <= d.maxArity;
}

} // namespace builtins

#endif // BUILTINS_BUILTIN_REGISTRY_H_
