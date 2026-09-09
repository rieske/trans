#include "StackMachine.h"

#include <stdexcept>

namespace codegen {

void StackMachine::emit(const Instruction& instruction) {
    switch (instruction.op) {
    case Op::Label:
        label(instruction.arg0);
        break;
    case Op::Jump:
        jump(instruction.cond, instruction.arg0, instruction.imm != 0);
        break;
    case Op::ValueCompare:
        compare(instruction.arg0, instruction.arg1, instruction.imm != 0);
        break;
    case Op::ZeroCompare:
        zeroCompare(instruction.arg0);
        break;
    case Op::AddressOf:
        addressOf(instruction.arg0, instruction.result);
        break;
    case Op::Dereference:
        dereference(instruction.arg0, instruction.arg1, instruction.result);
        break;
    case Op::IndexAddress:
        indexAddress(
                instruction.arg0, instruction.arg1, instruction.imm, instruction.result,
                instruction.baseMode);
        break;
    case Op::PointerOffset:
        pointerOffset(instruction.arg0, instruction.arg1, instruction.imm,
                instruction.result, instruction.pointerSubtract);
        break;
    case Op::PointerDiff:
        pointerDifference(
                instruction.arg0, instruction.arg1, instruction.imm, instruction.result);
        break;
    case Op::FieldAddress:
        fieldAddress(
                instruction.arg0, instruction.imm, instruction.result, instruction.baseMode);
        break;
    case Op::CopyPart:
        copyPart(instruction.arg0, instruction.result, instruction.imm);
        break;
    case Op::FunctionAddress:
        functionAddress(instruction.arg0, instruction.result);
        break;
    case Op::UnaryMinus:
        unaryMinus(instruction.arg0, instruction.result);
        break;
    case Op::UnaryNot:
        unaryNot(instruction.arg0, instruction.result);
        break;
    case Op::Assign:
        assign(instruction.arg0, instruction.result);
        break;
    case Op::Widen:
        widenInteger(instruction.arg0, instruction.result, instruction.imm != 0);
        break;
    case Op::AssignConstant:
        assignConstant(instruction.arg0, instruction.result, instruction.arg1);
        break;
    case Op::AssignLabelAddress:
        assignLabelAddress(instruction.arg0, instruction.result);
        break;
    case Op::LvalueAssign:
        lvalueAssign(instruction.arg0, instruction.result);
        break;
    case Op::Argument:
        procedureArgument(instruction.arg0);
        break;
    case Op::Call:
        if (instruction.callIndirect) {
            callProcedureIndirect(instruction.arg0, instruction.memoryReturnDest);
        } else {
            callProcedure(instruction.arg0, instruction.memoryReturnDest);
        }
        break;
    case Op::Return:
        returnFromProcedure(instruction.arg0);
        break;
    case Op::VoidReturn:
        returnFromProcedure();
        break;
    case Op::Retrieve:
        retrieveProcedureReturnValue(instruction.result, instruction.memoryReturn);
        break;
    case Op::Xor:
        xorCommand(instruction.arg0, instruction.arg1, instruction.result);
        break;
    case Op::Or:
        orCommand(instruction.arg0, instruction.arg1, instruction.result);
        break;
    case Op::And:
        andCommand(instruction.arg0, instruction.arg1, instruction.result);
        break;
    case Op::Add:
        add(instruction.arg0, instruction.arg1, instruction.result);
        break;
    case Op::Sub:
        sub(instruction.arg0, instruction.arg1, instruction.result);
        break;
    case Op::Mul:
        mul(instruction.arg0, instruction.arg1, instruction.result);
        break;
    case Op::Div:
        div(instruction.arg0, instruction.arg1, instruction.result,
                instruction.imm != 0);
        break;
    case Op::Mod:
        mod(instruction.arg0, instruction.arg1, instruction.result,
                instruction.imm != 0);
        break;
    case Op::Inc:
        inc(instruction.arg0, instruction.imm);
        break;
    case Op::Dec:
        dec(instruction.arg0, instruction.imm);
        break;
    case Op::Shl:
        shl(instruction.arg0, instruction.arg1, instruction.result);
        break;
    case Op::Shr:
        shr(instruction.arg0, instruction.arg1, instruction.result,
                instruction.imm != 0);
        break;
    case Op::VaStart:
        vaStart(instruction.arg0, instruction.arg1);
        break;
    case Op::VaArg:
        vaArg(instruction.arg0, instruction.result);
        break;
    case Op::VaCopy:
        vaCopy(instruction.arg0, instruction.arg1);
        break;
    case Op::VaEnd:
        vaEnd();
        break;
    case Op::Bswap:
        bswap(instruction.arg0, instruction.result, instruction.imm);
        break;
    case Op::Ctz:
        ctz(instruction.arg0, instruction.result, instruction.imm);
        break;
    case Op::Alloca:
        allocaBytes(instruction.arg0, instruction.result);
        break;
    default:
        throw std::logic_error { "StackMachine::emit: unhandled Op" };
    }
    finishInstruction();
}

} // namespace codegen
