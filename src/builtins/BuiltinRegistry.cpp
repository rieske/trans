#include "BuiltinRegistry.h"

#include "builtins/BswapTable.h"

namespace builtins {

namespace {

BuiltinDescriptor fillBuiltin(symbols::BuiltinPlan plan, int minArity, int maxArity,
        type::Type returnType, type::Type syntheticArgType = type::unsignedLong()) {
    BuiltinDescriptor d;
    d.kind = SimpleBuiltin { std::move(plan) };
    d.minArity = minArity;
    d.maxArity = maxArity;
    d.returnType = std::move(returnType);
    d.syntheticArgType = std::move(syntheticArgType);
    return d;
}

BuiltinDescriptor fillCallAs(std::string callee, int minArity, int maxArity,
        type::Type returnType, type::Type syntheticArgType = type::unsignedLong()) {
    BuiltinDescriptor d;
    d.kind = SyntheticCall { std::move(callee) };
    d.minArity = minArity;
    d.maxArity = maxArity;
    d.returnType = std::move(returnType);
    d.syntheticArgType = std::move(syntheticArgType);
    return d;
}

BuiltinDescriptor fillKind(BuiltinKind kind, int minArity, int maxArity, type::Type returnType) {
    BuiltinDescriptor d;
    d.kind = std::move(kind);
    d.minArity = minArity;
    d.maxArity = maxArity;
    d.returnType = std::move(returnType);
    return d;
}

BuiltinDescriptor makeConstantP() {
    return fillBuiltin(symbols::ConstantZeroPlan {}, 1, 1, type::signedInteger(), type::signedInteger());
}

BuiltinDescriptor makeAlloca() {
    return fillCallAs("malloc", 1, 1, type::pointer(type::voidType()), type::unsignedLong());
}

BuiltinDescriptor makeCtz() {
    return fillBuiltin(symbols::CtzPlan {}, 1, 1,
            type::signedInteger(), type::unsignedLong());
}

BuiltinDescriptor makeVaStart() {
    return fillBuiltin(symbols::VaStartPlan {}, 1, 2, type::voidType());
}

BuiltinDescriptor makeVaEnd() {
    return fillBuiltin(symbols::VaEndPlan {}, 1, 1, type::voidType());
}

BuiltinDescriptor makeVaCopy() {
    return fillBuiltin(symbols::VaCopyPlan {}, 2, 2, type::voidType());
}

BuiltinDescriptor makeVaArg() {
    return fillKind(TypeNameReturn {}, 1, 1, type::signedInteger());
}

struct BuiltinNameEntry {
    const char* name;
    BuiltinDescriptor (*make)();
};

// Non-bswap product builtins. Bswap names come only from kBswapBuiltins.
constexpr BuiltinNameEntry kBuiltinNames[] = {
        { "__builtin_constant_p", makeConstantP },
        { "__builtin_alloca", makeAlloca },
        { "__builtin_ctz", makeCtz },
        { "__builtin_ctzl", makeCtz },
        { "__builtin_ctzll", makeCtz },
        { "__builtin_va_start", makeVaStart },
        { "__builtin_c23_va_start", makeVaStart },
        { "__builtin_va_end", makeVaEnd },
        { "__builtin_va_copy", makeVaCopy },
        { "__builtin_va_arg", makeVaArg },
};

} // namespace

std::optional<BuiltinDescriptor> lookupBuiltin(const std::string& designatorName) {
    for (const auto& e : kBuiltinNames) {
        if (designatorName == e.name) {
            return e.make();
        }
    }
    for (const auto& bswap : kBswapBuiltins) {
        if (designatorName == bswap.name) {
            type::Type value = bswapValueType(bswap.widthBytes);
            return fillBuiltin(symbols::BswapPlan { bswap.widthBytes }, 1, 1, value, value);
        }
    }
    return std::nullopt;
}

} // namespace builtins
