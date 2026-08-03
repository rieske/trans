#include "SymbolRefs.h"

#include <stdexcept>

namespace codegen {

void collectSymbolRefs(const Instruction& instruction, SymbolRefs& refs) {
    switch (instruction.op) {
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
    case Op::ValueCompare:
        refs.addUse(instruction.arg0);
        refs.addUse(instruction.arg1);
        refs.addDef(instruction.result);
        return;
    case Op::IndexAddress:
        refs.addUse(instruction.arg0);
        refs.addUse(instruction.arg1);
        refs.addDef(instruction.result);
        if (symbols::addressBaseUsesLea(instruction.baseMode)) {
            refs.addressOfBase = instruction.arg0;
        }
        return;
    case Op::Assign:
        refs.addUse(instruction.arg0);
        refs.addDef(instruction.result);
        refs.assignCopyFrom = instruction.arg0;
        return;
    case Op::UnaryMinus:
    case Op::UnaryNot:
    case Op::CopyPart:
    case Op::Widen:
    case Op::Bswap:
    case Op::Ctz:
    case Op::Alloca:
        refs.addUse(instruction.arg0);
        refs.addDef(instruction.result);
        return;
    case Op::AssignConstant:
    case Op::AssignLabelAddress:
    case Op::FunctionAddress:
        // Constant / pool label / function label is not a live Value.
        refs.addDef(instruction.result);
        return;
    case Op::AddressOf:
        refs.addUse(instruction.arg0);
        refs.addDef(instruction.result);
        refs.addressOfBase = instruction.arg0;
        return;
    case Op::LvalueAssign:
        refs.addUse(instruction.arg0);
        refs.addUse(instruction.result);
        return;
    case Op::Dereference:
        refs.addUse(instruction.arg0);
        refs.addDef(instruction.result);
        return;
    case Op::FieldAddress:
        refs.addUse(instruction.arg0);
        refs.addDef(instruction.result);
        if (symbols::addressBaseUsesLea(instruction.baseMode)) {
            refs.addressOfBase = instruction.arg0;
        }
        return;
    case Op::Inc:
    case Op::Dec:
    case Op::Truncate:
        refs.addUse(instruction.arg0);
        refs.addDef(instruction.arg0);
        return;
    case Op::ZeroCompare:
        refs.addUse(instruction.arg0);
        return;
    case Op::Jump:
    case Op::Label:
    case Op::VoidReturn:
        return;
    case Op::Argument:
        refs.addUse(instruction.arg0);
        refs.isParam = true;
        return;
    case Op::Call:
        refs.isCall = true;
        if (instruction.callIndirect) {
            refs.addUse(instruction.arg0);
        }
        refs.addUse(instruction.memoryReturnDest);
        return;
    case Op::Retrieve:
        refs.addDef(instruction.result);
        return;
    case Op::Return:
        refs.addUse(instruction.arg0);
        return;
    case Op::VaStart:
        refs.addUse(instruction.arg0);
        return;
    case Op::VaArg:
        refs.addUse(instruction.arg0);
        refs.addDef(instruction.result);
        return;
    case Op::VaCopy:
        refs.addUse(instruction.arg0);
        refs.addUse(instruction.arg1);
        return;
    }
    throw std::logic_error { "collectSymbolRefs: unhandled Op" };
}

} // namespace codegen
