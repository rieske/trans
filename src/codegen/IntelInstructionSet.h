#ifndef INTEL_INSTRUCTIONSET_H_
#define INTEL_INSTRUCTIONSET_H_

#include <string>

#include "InstructionSet.h"

namespace codegen {

class IntelInstructionSet: public InstructionSet {
public:
    virtual ~IntelInstructionSet();

    std::string preamble(std::map<std::string, std::string> constants) const override;

    std::string call(std::string procedureName) const override;

    std::string push(const Register& reg) const override;
    std::string pop(const Register& reg) const override;

    std::string add(const Register& reg, int constant) const override;
    std::string sub(const Register& reg, int constant) const override;

    std::string lea(const Register& base, int offset, const Register& target) const override;

    std::string not_(const Register& reg) const override;

    std::string mov(const Register& from, const Register& memoryBase, int memoryOffset) const override;
    std::string mov(const Register& from, const Register& to) const override;
    std::string mov(const Register& memoryBase, int memoryOffset, const Register& to) const override;
    std::string mov(std::string constant, const Register& memoryBase, int memoryOffset) const override;
    std::string mov(std::string constant, const Register& to) const override;

    std::string cmp(const Register& leftArgument, const Register& memoryBase, int memoryOffset) const override;
    std::string cmp(const Register& leftArgument, const Register& rightArgument) const override;
    std::string cmp(const Register& memoryBase, int memoryOffset, const Register& rightArgument) const override;
    std::string cmp(const Register& argument, int constant) const override;
    std::string cmp(const Register& memoryBase, int memoryOffset, int constant) const override;

    std::string label(std::string name) const override;
    std::string jmp(std::string label) const override;
    std::string je(std::string label) const override;
    std::string jne(std::string label) const override;
    std::string jg(std::string label) const override;
    std::string jl(std::string label) const override;
    std::string jge(std::string label) const override;
    std::string jle(std::string label) const override;

    std::string syscall() const override;
    std::string leave() const override;
    std::string ret() const override;

    std::string xor_(const Register& operand, const Register& result) const override;
    std::string xor_(const Register& operandBase, int operandOffset, const Register& result) const override;

    std::string or_(const Register& operand, const Register& result) const override;
    std::string or_(const Register& operandBase, int operandOffset, const Register& result) const override;

    std::string and_(const Register& operand, const Register& result) const override;
    std::string and_(const Register& operandBase, int operandOffset, const Register& result) const override;

    std::string shl(const Register& result) const override;
    //std::string shl(std::string constant, const Register& result) const override;
    std::string shr(const Register& result) const override;
    //std::string shr(std::string constant, const Register& result) const override;

    std::string add(const Register& operand, const Register& result) const override;
    std::string add(const Register& operandBase, int operandOffset, const Register& result) const override;

    std::string sub(const Register& operand, const Register& result) const override;
    std::string sub(const Register& operandBase, int operandOffset, const Register& result) const override;

    std::string imul(const Register& operand) const override;
    std::string imul(const Register& operandBase, int operandOffset) const override;

    std::string idiv(const Register& operand) const override;
    std::string idiv(const Register& operandBase, int operandOffset) const override;

    std::string inc(const Register& operand) const override;
    std::string inc(const Register& operandBase, int operandOffset) const override;

    std::string dec(const Register& operand) const override;
    std::string dec(const Register& operandBase, int operandOffset) const override;

    std::string neg(const Register& operand) const override;
};

} // namespace codegen

#endif // INTEL_INSTRUCTIONSET_H_
