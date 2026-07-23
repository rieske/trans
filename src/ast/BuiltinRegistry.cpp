#include "BuiltinRegistry.h"

namespace ast {


FunctionCall::BuiltinKind builtinKindForName(const std::string& name) {
    using K = FunctionCall::BuiltinKind;
    if (name == "__builtin_constant_p") return K::ConstantP;
    if (name == "__builtin_alloca") return K::AllocaAsMalloc;
    if (name == "__builtin_bswap16") return K::Bswap16;
    if (name == "__builtin_bswap32") return K::Bswap32;
    if (name == "__builtin_bswap64") return K::Bswap64;
    if (name == "__builtin_ctz" || name == "__builtin_ctzl" || name == "__builtin_ctzll") {
        return K::Ctz;
    }
    if (name == "__builtin_va_start" || name == "__builtin_c23_va_start") {
        return K::VaStart;
    }
    if (name == "__builtin_va_end") return K::VaEnd;
    if (name == "__builtin_va_copy") return K::VaCopy;
    if (name == "__builtin_va_arg") return K::VaArg;
    return K::None;
}

std::optional<BuiltinDescriptor> lookupBuiltin(
        FunctionCall::BuiltinKind existingKind,
        const std::string& designatorName,
        const type::Type* vaArgResultType) {
    using K = FunctionCall::BuiltinKind;
    K kind = existingKind;
    if (kind == K::None) {
        kind = builtinKindForName(designatorName);
    }
    if (kind == K::None) {
        return std::nullopt;
    }

    BuiltinDescriptor d;
    d.kind = kind;
    d.minArity = 1;
    d.maxArity = 1;
    d.returnType = type::signedInteger();
    d.syntheticArgType = type::unsignedLong();

    switch (kind) {
    case K::ConstantP:
        d.lowering = BuiltinLowering::ConstantZero;
        d.returnType = type::signedInteger();
        d.syntheticArgType = type::signedInteger();
        break;
    case K::AllocaAsMalloc:
        d.lowering = BuiltinLowering::CallAs;
        d.returnType = type::pointer(type::voidType());
        d.syntheticArgType = type::unsignedLong();
        d.syntheticName = "malloc";
        break;
    case K::Bswap16:
        d.lowering = BuiltinLowering::BuiltinOp;
        d.returnType = type::unsignedShort();
        d.syntheticArgType = type::unsignedShort();
        break;
    case K::Bswap32:
        d.lowering = BuiltinLowering::BuiltinOp;
        d.returnType = type::unsignedInteger();
        d.syntheticArgType = type::unsignedInteger();
        break;
    case K::Bswap64:
        d.lowering = BuiltinLowering::BuiltinOp;
        d.returnType = type::unsignedLong();
        d.syntheticArgType = type::unsignedLong();
        break;
    case K::Ctz:
        d.lowering = BuiltinLowering::BuiltinOp;
        d.returnType = type::signedInteger();
        d.syntheticArgType = type::unsignedLong();
        break;
    case K::VaStart:
        d.lowering = BuiltinLowering::VaStart;
        d.returnType = type::voidType();
        d.minArity = 1;
        d.maxArity = 2;
        break;
    case K::VaEnd:
        d.lowering = BuiltinLowering::VaEnd;
        d.returnType = type::voidType();
        d.minArity = 1;
        d.maxArity = 1;
        break;
    case K::VaCopy:
        d.lowering = BuiltinLowering::VaCopy;
        d.returnType = type::voidType();
        d.minArity = 2;
        d.maxArity = 2;
        break;
    case K::VaArg:
        d.lowering = BuiltinLowering::VaArg;
        d.returnType = vaArgResultType ? *vaArgResultType : type::signedInteger();
        d.minArity = 1;
        d.maxArity = 1;
        break;
    default:
        break;
    }
    return d;
}

} // namespace ast
