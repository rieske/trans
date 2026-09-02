#include "Instruction.h"

#include "SysVCallConv.h"
#include "ValueKind.h"
#include "types/ObjectAbi.h"
#include "types/SysVClassify.h"

#include <stdexcept>
#include <string>

namespace codegen {

void internProcedureTemps(IrStringTable& strings, Procedure& procedure) {
    if (procedure.memoryReturn) {
        procedure.sretId = strings.intern(type::object_abi::SRET_SYMBOL_NAME);
    }
    if (!procedure.variadic) {
        return;
    }
    procedure.vaGpHomes.resize(SYSV_INTEGER_ARG_REGS, kNoSymbol);
    procedure.vaXmmHomes.resize(SYSV_SSE_ARG_REGS, kNoSymbol);
    for (std::size_t i = 0; i < SYSV_INTEGER_ARG_REGS; ++i) {
        procedure.vaGpHomes[i] = strings.intern(vaGpHomeName(i));
    }
    for (std::size_t i = 0; i < SYSV_SSE_ARG_REGS; ++i) {
        procedure.vaXmmHomes[i] = strings.intern(vaXmmHomeName(i));
    }
}

int addFrameTemp(IrStringTable& strings, Procedure& procedure, const type::Type& type) {
    int n = 0;
    std::string name;
    do {
        name = "__t" + std::to_string(n++);
    } while (strings.find(name) != kNoSymbol);
    Value scratch {
            strings.intern(name),
            0,
            valueKindFromCType(type),
            type.getSize(),
            type::sysv::classify(type)
    };
    scratch.markExpressionTemp();
    const int scratchId = scratch.id();
    procedure.frame.locals.push_back(std::move(scratch));
    return scratchId;
}

namespace {

enum Live : unsigned {
    FArg0 = 1u << 0,
    FArg1 = 1u << 1,
    FResult = 1u << 2,
    FImm = 1u << 3,
    FCond = 1u << 4,
    FBaseMode = 1u << 5,
    FCallIndirect = 1u << 6,
    FPointerSubtract = 1u << 7,
    FMemoryReturnDest = 1u << 8,
    FMemoryReturn = 1u << 9,
};

struct OpContract {
    InstructionClass kind;
    unsigned live;
};

OpContract opContract(Op op) {
    switch (op) {
    case Op::Add:
    case Op::Sub:
    case Op::Mul:
    case Op::And:
    case Op::Or:
    case Op::Xor:
    case Op::Shl:
    case Op::AssignConstant:
    case Op::Dereference:
        return { InstructionClass::Ordinary, FArg0 | FArg1 | FResult };
    case Op::Div:
    case Op::Mod:
    case Op::Shr:
    case Op::PointerDiff:
        return { InstructionClass::Ordinary, FArg0 | FArg1 | FResult | FImm };
    case Op::UnaryMinus:
    case Op::UnaryNot:
    case Op::Assign:
    case Op::LvalueAssign:
    case Op::AddressOf:
    case Op::AssignLabelAddress:
    case Op::FunctionAddress:
    case Op::Alloca:
    case Op::VaArg:
        return { InstructionClass::Ordinary, FArg0 | FResult };
    case Op::Inc:
    case Op::Dec:
        return { InstructionClass::Ordinary, FArg0 | FImm };
    case Op::Widen:
    case Op::Bswap:
    case Op::Ctz:
        return { InstructionClass::Ordinary, FArg0 | FResult | FImm };
    case Op::IndexAddress:
        return { InstructionClass::Ordinary, FArg0 | FArg1 | FResult | FImm | FBaseMode };
    case Op::FieldAddress:
        return { InstructionClass::Ordinary, FArg0 | FResult | FImm | FBaseMode };
    case Op::CopyPart:
        return { InstructionClass::Ordinary, FArg0 | FResult | FImm };
    case Op::ValueCompare:
        return { InstructionClass::Ordinary, FArg0 | FArg1 | FImm };
    case Op::PointerOffset:
        return { InstructionClass::Ordinary, FArg0 | FArg1 | FResult | FImm | FPointerSubtract };
    case Op::ZeroCompare:
    case Op::Argument:
        return { InstructionClass::Ordinary, FArg0 };
    case Op::Label:
        return { InstructionClass::Label, FArg0 };
    case Op::Jump:
        return { InstructionClass::Terminator, FArg0 | FCond | FImm };
    case Op::Return:
        return { InstructionClass::Terminator, FArg0 };
    case Op::Call:
        return { InstructionClass::Ordinary, FArg0 | FCallIndirect | FMemoryReturnDest };
    case Op::Retrieve:
        return { InstructionClass::Ordinary, FResult | FMemoryReturn };
    case Op::VoidReturn:
        return { InstructionClass::Terminator, 0 };
    case Op::VaEnd:
        return { InstructionClass::Ordinary, 0 };
    case Op::VaStart:
    case Op::VaCopy:
        return { InstructionClass::Ordinary, FArg0 | FArg1 };
    }
    throw std::logic_error { "opContract: unhandled Op" };
}

template<typename T>
void rejectIfUnused(unsigned live, unsigned bit, const T& value, const T& def) {
    if (!(live & bit) && value != def) {
        throw std::logic_error { "unused Instruction field set" };
    }
}

} // namespace

InstructionClass instructionClass(Op op) {
    return opContract(op).kind;
}

bool instructionTransfersControl(const Instruction& instruction) {
    return instructionClass(instruction.op) == InstructionClass::Terminator;
}

void validateInstruction(const Instruction& instruction) {
    const unsigned live = opContract(instruction.op).live;
    const Instruction defaults {};
    rejectIfUnused(live, FArg0, instruction.arg0, defaults.arg0);
    rejectIfUnused(live, FArg1, instruction.arg1, defaults.arg1);
    rejectIfUnused(live, FResult, instruction.result, defaults.result);
    rejectIfUnused(live, FImm, instruction.imm, defaults.imm);
    rejectIfUnused(live, FCond, instruction.cond, defaults.cond);
    rejectIfUnused(live, FBaseMode, instruction.baseMode, defaults.baseMode);
    rejectIfUnused(live, FCallIndirect, instruction.callIndirect, defaults.callIndirect);
    rejectIfUnused(live, FPointerSubtract, instruction.pointerSubtract, defaults.pointerSubtract);
    rejectIfUnused(live, FMemoryReturnDest, instruction.memoryReturnDest, defaults.memoryReturnDest);
    rejectIfUnused(live, FMemoryReturn, instruction.memoryReturn, defaults.memoryReturn);
}

} // namespace codegen
