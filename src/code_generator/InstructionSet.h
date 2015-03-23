#ifndef INSTRUCTIONSET_H_
#define INSTRUCTIONSET_H_

#include <string>

#include "Register.h"

namespace codegen {

class InstructionSet {
public:
    InstructionSet();
    virtual ~InstructionSet() = default;

    std::string preamble() const;

    std::string mainProcedure() const;
    std::string call(std::string procedureName) const;

    std::string push(const Register& reg) const;
    std::string pop(const Register& reg) const;

    std::string add(const Register& reg, int constant) const;
    std::string sub(const Register& reg, int constant) const;

    std::string negate(const Register& reg) const;

    std::string mov(const Register& from, const Register& memoryBase, int memoryOffset) const;
    std::string mov(const Register& from, const Register& to) const;
    std::string mov(const Register& memoryBase, int memoryOffset, const Register& to) const;
    std::string mov(std::string constant, const Register& memoryBase, int memoryOffset) const;
    std::string mov(std::string constant, const Register& to) const;

    std::string cmp(const Register& leftArgument, const Register& memoryBase, int memoryOffset) const;
    std::string cmp(const Register& leftArgument, const Register& rightArgument) const;
    std::string cmp(const Register& memoryBase, int memoryOffset, const Register& rightArgument) const;
    std::string cmp(const Register& argument, int constant) const;
    std::string cmp(const Register& memoryBase, int memoryOffset, int constant) const;

    std::string label(std::string name) const;
    std::string jmp(std::string label) const;
    std::string je(std::string label) const;
    std::string jne(std::string label) const;
    std::string jg(std::string label) const;
    std::string jl(std::string label) const;
    std::string jge(std::string label) const;
    std::string jle(std::string label) const;

    std::string syscall() const;
    std::string ret() const;

    std::string xor_(const Register& operand, const Register& result) const;
    std::string xor_(const Register& operandBase, int operandOffset, const Register& result) const;

    std::string or_(const Register& operand, const Register& result) const;
    std::string or_(const Register& operandBase, int operandOffset, const Register& result) const;

    std::string and_(const Register& operand, const Register& result) const;
    std::string and_(const Register& operandBase, int operandOffset, const Register& result) const;

    std::string add(const Register& operand, const Register& result) const;
    std::string add(const Register& operandBase, int operandOffset, const Register& result) const;

    std::string sub(const Register& operand, const Register& result) const;
    std::string sub(const Register& operandBase, int operandOffset, const Register& result) const;

    std::string imul(const Register& operand) const;
    std::string imul(const Register& operandBase, int operandOffset) const;

    std::string idiv(const Register& operand) const;
    std::string idiv(const Register& operandBase, int operandOffset) const;

    std::string inc(const Register& operand) const;
    std::string inc(const Register& operandBase, int operandOffset) const;

    std::string dec(const Register& operand) const;
    std::string dec(const Register& operandBase, int operandOffset) const;
};

} /* namespace code_generator */

#endif /* INSTRUCTIONSET_H_ */
