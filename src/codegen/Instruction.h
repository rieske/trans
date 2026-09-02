#ifndef CODEGEN_INSTRUCTION_H_
#define CODEGEN_INSTRUCTION_H_

#include <ostream>
#include <string>
#include <vector>

#include "codegen/IrStringTable.h"
#include "codegen/JumpCondition.h"
#include "codegen/Value.h"
#include "symbols/AddressPlan.h"

namespace type {
class Type;
}

namespace codegen {

// Mid-end opcode stream. Procedure::body stays the persistent linear
// form. A CFG exists only inside the mid-end. It does not grow
// StackMachine. New mid-level semantics go through ir:: builders +
// runIrPasses. New x86-only tricks go in StackMachine.
//
// Live fields only; unused fields stay at Instruction defaults.
enum class Op {
    Add,                 // arg0 arg1 result
    Sub,                 // arg0 arg1 result
    Mul,                 // arg0 arg1 result
    Div,                 // arg0 arg1 result imm(signed)
    Mod,                 // arg0 arg1 result imm(signed)
    And,                 // arg0 arg1 result
    Or,                  // arg0 arg1 result
    Xor,                 // arg0 arg1 result
    Shl,                 // arg0 arg1 result
    Shr,                 // arg0 arg1 result imm(arithmetic)
    UnaryMinus,          // arg0 result
    UnaryNot,            // arg0 result
    Inc,                 // arg0 imm(step)
    Dec,                 // arg0 imm(step)
    Assign,              // arg0 result
    Widen,               // arg0 result imm(signHighWord)
    AssignConstant,      // arg0 result arg1(high, optional)
    AssignLabelAddress,  // arg0(label) result
    LvalueAssign,        // arg0 result
    AddressOf,           // arg0 result
    Dereference,         // arg0 arg1(lvalue) result
    IndexAddress,        // arg0 arg1 result imm(stride) baseMode
    FieldAddress,        // arg0 result imm(offset) baseMode
    CopyPart,            // arg0 result imm(byteOffset)
    PointerOffset,       // arg0 arg1 result imm(elemSize) pointerSubtract
    PointerDiff,         // arg0 arg1 result imm(elemSize)
    FunctionAddress,     // arg0 result
    ValueCompare,        // arg0 arg1 imm(signedRel)
    ZeroCompare,         // arg0
    Jump,                // arg0(label) cond imm(signedRel)
    Label,               // arg0
    Argument,            // arg0
    Call,                // arg0 callIndirect memoryReturnDest
    Retrieve,            // result memoryReturn
    Return,              // arg0
    VoidReturn,          // (none)
    VaStart,             // arg0 arg1(last, optional)
    VaArg,               // arg0 result
    VaEnd,               // (none)
    VaCopy,              // arg0 arg1
    Bswap,               // arg0 result imm(width)
    Ctz,                 // arg0 result imm(width)
    Alloca,              // arg0 result
};

struct ProcedureFrame {
    std::vector<Value> locals;
    std::vector<Value> arguments;
};

// Sparse fields: live set is per Op (see enumerator comments).
// Unused fields must stay at these defaults (validateInstruction).
// Value, label, constant, and function operands are ids into IrStringTable.
struct Instruction {
    Op op {};
    int arg0 { kNoSymbol };
    int arg1 { kNoSymbol };
    int result { kNoSymbol };
    int imm { 0 };
    JumpCondition cond { JumpCondition::UNCONDITIONAL };
    symbols::AddressBaseMode baseMode { symbols::AddressBaseMode::LeaObject };
    bool callIndirect { false };
    bool pointerSubtract { false };
    int memoryReturnDest { kNoSymbol };
    bool memoryReturn { false };
};

struct Procedure {
    int name { kNoSymbol };
    ProcedureFrame frame;
    // Linearization of a CFG that exists only inside the mid-end.
    // Implicit blocks: (label or start) ordinary* terminator?.
    // Fall-through into a following Label, or after a conditional Jump,
    // is an implicit CFG edge.
    std::vector<Instruction> body;
    bool memoryReturn { false };
    bool variadic { false };
    bool exported { true };
    int sretId { kNoSymbol };
    std::vector<int> vaGpHomes;
    std::vector<int> vaXmmHomes;
};

struct IntermediateRepresentation {
    IrStringTable strings;
    std::vector<Procedure> procedures;
};

void internProcedureTemps(IrStringTable& strings, Procedure& procedure);
int addFrameTemp(IrStringTable& strings, Procedure& procedure, const type::Type& type);

enum class InstructionClass { Label, Terminator, Ordinary };

InstructionClass instructionClass(Op op);
bool instructionTransfersControl(const Instruction& instruction);
void validateInstruction(const Instruction& instruction);

void print(std::ostream& stream, const Instruction& instruction, const IrStringTable& strings);
void print(std::ostream& stream, const Procedure& procedure, const IrStringTable& strings);
void print(std::ostream& stream, const IntermediateRepresentation& ir);

std::string toString(const IntermediateRepresentation& ir);

std::ostream& operator<<(std::ostream& stream, const IntermediateRepresentation& ir);

} // namespace codegen

#include "codegen/IrBuilders.h"

#endif // CODEGEN_INSTRUCTION_H_
