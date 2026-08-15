#include "Instruction.h"

#include "SysVCallConv.h"
#include "types/ObjectAbi.h"

#include <stdexcept>

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

bool instructionTransfersControl(const Instruction& instruction) {
    switch (instruction.op) {
    case Op::Jump:
    case Op::Return:
    case Op::VoidReturn:
        return true;
    case Op::Add:
    case Op::Sub:
    case Op::Mul:
    case Op::Div:
    case Op::Mod:
    case Op::And:
    case Op::Or:
    case Op::Xor:
    case Op::Shl:
    case Op::Shr:
    case Op::UnaryMinus:
    case Op::UnaryNot:
    case Op::Inc:
    case Op::Dec:
    case Op::Assign:
    case Op::Widen:
    case Op::AssignConstant:
    case Op::AssignLabelAddress:
    case Op::LvalueAssign:
    case Op::AddressOf:
    case Op::Dereference:
    case Op::IndexAddress:
    case Op::FieldAddress:
    case Op::CopyPart:
    case Op::PointerOffset:
    case Op::PointerDiff:
    case Op::FunctionAddress:
    case Op::ValueCompare:
    case Op::ZeroCompare:
    case Op::Label:
    case Op::Argument:
    case Op::Call:
    case Op::Retrieve:
    case Op::VaStart:
    case Op::VaArg:
    case Op::VaEnd:
    case Op::VaCopy:
    case Op::Bswap:
    case Op::Ctz:
    case Op::Alloca:
        return false;
    }
    throw std::logic_error { "instructionTransfersControl: unhandled Op" };
}

} // namespace codegen
