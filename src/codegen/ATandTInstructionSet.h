#ifndef ATANDTINSTRUCTIONSET_H_
#define ATANDTINSTRUCTIONSET_H_

#include "InstructionSet.h"

namespace codegen {

class ATandTInstructionSet: public InstructionSet {
public:
    virtual ~ATandTInstructionSet();

    std::string preamble(const std::map<std::string, std::string>& constants,
            const std::vector<GlobalVariable>& globalVariables = {}) const override;

    std::string call(std::string procedureName) const override;

    std::string push(const Register& reg) const override;
    std::string pop(const Register& reg) const override;

    std::string add(const Register& reg, int constant) const override;
    std::string sub(const Register& reg, int constant) const override;

    std::string lea(const MemoryOperand& source, const Register& target) const override;

    std::string not_(const Register& reg) const override;

    std::string mov(const Register& source, const MemoryOperand& destination) const override;
    std::string mov(const Register& source, const Register& destination) const override;
    std::string mov(const MemoryOperand& source, const Register& destination) const override;
    std::string mov(std::string constant, const MemoryOperand& destination) const override;
    std::string mov(std::string constant, const Register& destination) const override;

    std::string cmp(const Register& leftArgument, const MemoryOperand& rightArgument) const override;
    std::string cmp(const Register& leftArgument, const Register& rightArgument) const override;
    std::string cmp(const MemoryOperand& leftArgument, const Register& rightArgument) const override;
    std::string cmp(const Register& argument, int constant) const override;
    std::string cmp(const MemoryOperand& leftArgument, int constant) const override;

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
    std::string xor_(const MemoryOperand& operand, const Register& result) const override;

    std::string or_(const Register& operand, const Register& result) const override;
    std::string or_(const MemoryOperand& operand, const Register& result) const override;

    std::string and_(const Register& operand, const Register& result) const override;
    std::string and_(const MemoryOperand& operand, const Register& result) const override;

    std::string shl(const Register& result) const override;
    //std::string shl(std::string constant, const Register& result) const override;
    std::string shr(const Register& result) const override;
    //std::string shr(std::string constant, const Register& result) const override;

    std::string add(const Register& operand, const Register& result) const override;
    std::string add(const MemoryOperand& operand, const Register& result) const override;

    std::string sub(const Register& operand, const Register& result) const override;
    std::string sub(const MemoryOperand& operand, const Register& result) const override;

    std::string imul(const Register& operand) const override;
    std::string imul(const MemoryOperand& operand) const override;

    std::string idiv(const Register& operand) const override;
    std::string idiv(const MemoryOperand& operand) const override;

    std::string inc(const Register& operand) const override;
    std::string inc(const MemoryOperand& operand) const override;

    std::string dec(const Register& operand) const override;
    std::string dec(const MemoryOperand& operand) const override;

    std::string neg(const Register& operand) const override;
};

} // namespace codegen

#endif // ATANDTINSTRUCTIONSET_H_
