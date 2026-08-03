#include "BuiltinRegistry.h"

namespace builtins {

namespace {

BuiltinDescriptor fillBuiltin(symbols::BuiltinPlan plan, int minArity, int maxArity,
        type::Type returnType, type::Type syntheticArgType = type::unsignedLong()) {
    BuiltinDescriptor d;
    d.builtinPlan = std::move(plan);
    d.minArity = minArity;
    d.maxArity = maxArity;
    d.returnType = std::move(returnType);
    d.syntheticArgType = std::move(syntheticArgType);
    return d;
}

BuiltinDescriptor fillCallAs(std::string callee, int minArity, int maxArity,
        type::Type returnType, type::Type syntheticArgType = type::unsignedLong()) {
    BuiltinDescriptor d;
    d.syntheticCallee = std::move(callee);
    d.minArity = minArity;
    d.maxArity = maxArity;
    d.returnType = std::move(returnType);
    d.syntheticArgType = std::move(syntheticArgType);
    return d;
}

BuiltinDescriptor makeConstantP(const type::Type*) {
    return fillBuiltin(symbols::ConstantZeroPlan {}, 1, 1, type::signedInteger(), type::signedInteger());
}

BuiltinDescriptor makeAlloca(const type::Type*) {
    return fillCallAs("malloc", 1, 1, type::pointer(type::voidType()), type::unsignedLong());
}

BuiltinDescriptor makeBswap16(const type::Type*) {
    return fillBuiltin(symbols::BuiltinOpPlan { symbols::BuiltinOpKind::Bswap16 }, 1, 1,
            type::unsignedShort(), type::unsignedShort());
}

BuiltinDescriptor makeBswap32(const type::Type*) {
    return fillBuiltin(symbols::BuiltinOpPlan { symbols::BuiltinOpKind::Bswap32 }, 1, 1,
            type::unsignedInteger(), type::unsignedInteger());
}

BuiltinDescriptor makeBswap64(const type::Type*) {
    return fillBuiltin(symbols::BuiltinOpPlan { symbols::BuiltinOpKind::Bswap64 }, 1, 1,
            type::unsignedLong(), type::unsignedLong());
}

BuiltinDescriptor makeCtz(const type::Type*) {
    return fillBuiltin(symbols::BuiltinOpPlan { symbols::BuiltinOpKind::Ctz }, 1, 1,
            type::signedInteger(), type::unsignedLong());
}

BuiltinDescriptor makeVaStart(const type::Type*) {
    return fillBuiltin(symbols::VaStartPlan {}, 1, 2, type::voidType());
}

BuiltinDescriptor makeVaEnd(const type::Type*) {
    return fillBuiltin(symbols::VaEndPlan {}, 1, 1, type::voidType());
}

BuiltinDescriptor makeVaCopy(const type::Type*) {
    return fillBuiltin(symbols::VaCopyPlan {}, 2, 2, type::voidType());
}

BuiltinDescriptor makeVaArg(const type::Type* vaArgResultType) {
    return fillBuiltin(symbols::VaArgPlan {}, 1, 1,
            vaArgResultType ? *vaArgResultType : type::signedInteger());
}

struct BuiltinNameEntry {
    const char* name;
    BuiltinDescriptor (*make)(const type::Type* vaArgResultType);
};

constexpr BuiltinNameEntry kBuiltinNames[] = {
        { "__builtin_constant_p", makeConstantP },
        { "__builtin_alloca", makeAlloca },
        { "__builtin_bswap16", makeBswap16 },
        { "__builtin_bswap32", makeBswap32 },
        { "__builtin_bswap64", makeBswap64 },
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

std::optional<BuiltinDescriptor> lookupBuiltin(
        const std::string& designatorName,
        const type::Type* vaArgResultType) {
    for (const auto& e : kBuiltinNames) {
        if (designatorName == e.name) {
            return e.make(vaArgResultType);
        }
    }
    return std::nullopt;
}

} // namespace builtins
