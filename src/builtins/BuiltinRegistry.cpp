#include "BuiltinRegistry.h"

#include "builtins/BswapTable.h"
#include "builtins/CtzTable.h"

namespace builtins {

namespace {

BuiltinDescriptor fillBuiltin(symbols::BuiltinPlan plan, int minArity, int maxArity,
        type::Type returnType, type::Type argType = type::unsignedLong()) {
    BuiltinDescriptor d;
    d.kind = SimpleBuiltin { std::move(plan) };
    d.minArity = minArity;
    d.maxArity = maxArity;
    d.returnType = std::move(returnType);
    d.argType = std::move(argType);
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

BuiltinDescriptor makeAlloca() {
    return fillBuiltin(symbols::AllocaPlan {}, 1, 1,
            type::pointer(type::voidType()), type::unsignedLong());
}

BuiltinDescriptor makeCtz(int widthBytes) {
    return fillBuiltin(symbols::CtzPlan { widthBytes }, 1, 1,
            type::signedInteger(), ctzArgType(widthBytes));
}

BuiltinDescriptor makeBswap(int widthBytes) {
    type::Type value = bswapValueType(widthBytes);
    return fillBuiltin(symbols::BswapPlan { widthBytes }, 1, 1, value, value);
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
    bool installDesignator { false };
};

// Call-form names that are not width tables. Bswap/ctz come from kBswapBuiltins / kCtzBuiltins.
constexpr BuiltinNameEntry kBuiltinNames[] = {
        { "__builtin_alloca", makeAlloca, true },
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
            return makeBswap(bswap.widthBytes);
        }
    }
    for (const auto& ctz : kCtzBuiltins) {
        if (designatorName == ctz.name) {
            return makeCtz(ctz.widthBytes);
        }
    }
    return std::nullopt;
}

void forEachDesignatorBuiltin(
        const std::function<void(const char* name, const BuiltinDescriptor& desc)>& fn) {
    for (const auto& bswap : kBswapBuiltins) {
        fn(bswap.name, makeBswap(bswap.widthBytes));
    }
    for (const auto& ctz : kCtzBuiltins) {
        fn(ctz.name, makeCtz(ctz.widthBytes));
    }
    for (const auto& e : kBuiltinNames) {
        if (e.installDesignator) {
            fn(e.name, e.make());
        }
    }
}

} // namespace builtins
