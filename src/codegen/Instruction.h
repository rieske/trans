#ifndef CODEGEN_INSTRUCTION_H_
#define CODEGEN_INSTRUCTION_H_

#include <ostream>
#include <string>
#include <vector>

#include "codegen/JumpCondition.h"
#include "codegen/Value.h"
#include "symbols/AddressPlan.h"

namespace codegen {

enum class Op {
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    And,
    Or,
    Xor,
    Shl,
    Shr,
    UnaryMinus,
    UnaryNot,
    Inc,
    Dec,
    Assign,
    Widen,
    AssignConstant,
    AssignLabelAddress,
    LvalueAssign,
    AddressOf,
    Dereference,
    IndexAddress,
    FieldAddress,
    CopyPart,
    PointerOffset,
    PointerDiff,
    FunctionAddress,
    ValueCompare,
    ZeroCompare,
    Jump,
    Label,
    Argument,
    Call,
    Retrieve,
    Return,
    VoidReturn,
    VaStart,
    VaArg,
    VaEnd,
    VaCopy,
    Bswap,
    Ctz,
    Alloca,
};

struct ProcedureFrame {
    std::vector<Value> locals;
    std::vector<Value> arguments;
};

// Sparse fields: meaning depends on op (see ir:: builders and emit/print).
struct Instruction {
    Op op {};
    std::string arg0;
    std::string arg1;
    std::string result;
    int imm { 0 };
    JumpCondition cond { JumpCondition::UNCONDITIONAL };
    symbols::AddressBaseMode baseMode { symbols::AddressBaseMode::LeaObject };
    bool callIndirect { false };
    bool pointerSubtract { false };
    std::string memoryReturnDest;
    bool memoryReturn { false };
};

struct Procedure {
    std::string name;
    ProcedureFrame frame;
    std::vector<Instruction> body;
    bool memoryReturn { false };
    bool variadic { false };
    bool exported { true };
};

struct IntermediateRepresentation {
    std::vector<Procedure> procedures;
};

bool instructionTransfersControl(const Instruction& instruction);

void print(std::ostream& stream, const Instruction& instruction);
void print(std::ostream& stream, const Procedure& procedure);
void print(std::ostream& stream, const IntermediateRepresentation& ir);

std::string toString(const Instruction& instruction);
std::string toString(const Procedure& procedure);
std::string toString(const IntermediateRepresentation& ir);

std::ostream& operator<<(std::ostream& stream, const Instruction& instruction);
std::ostream& operator<<(std::ostream& stream, const Procedure& procedure);
std::ostream& operator<<(std::ostream& stream, const IntermediateRepresentation& ir);

} // namespace codegen

#include "codegen/IrBuilders.h"

#endif // CODEGEN_INSTRUCTION_H_
