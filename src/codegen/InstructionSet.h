#ifndef INSTRUCTIONSET_H_
#define INSTRUCTIONSET_H_

#include <string>
#include <map>
#include <vector>

#include "GlobalVariable.h"
#include "MemoryOperand.h"

namespace codegen {

class Register;

class InstructionSet {
public:
    virtual ~InstructionSet() = default;

    virtual std::string preamble(const std::map<std::string, std::string>& constants,
            const std::vector<GlobalVariable>& globalVariables = {}) const = 0;

    virtual std::string call(std::string procedureName) const = 0;

    virtual std::string push(const Register& reg) const = 0;
    virtual std::string pop(const Register& reg) const = 0;

    virtual std::string lea(const MemoryOperand& source, const Register& target) const = 0;

    virtual std::string add(const Register& reg, int constant) const = 0;
    virtual std::string sub(const Register& reg, int constant) const = 0;

    virtual std::string not_(const Register& reg) const = 0;

    virtual std::string mov(const Register& from, const MemoryOperand& destination) const = 0;
    virtual std::string mov(const Register& from, const Register& to) const = 0;
    virtual std::string mov(const MemoryOperand& source, const Register& to) const = 0;
    virtual std::string mov(std::string constant, const MemoryOperand& destination) const = 0;
    virtual std::string mov(std::string constant, const Register& to) const = 0;

    virtual std::string cmp(const Register& leftArgument, const MemoryOperand& rightArgument) const = 0;
    virtual std::string cmp(const Register& leftArgument, const Register& rightArgument) const = 0;
    virtual std::string cmp(const MemoryOperand& leftArgument, const Register& rightArgument) const = 0;
    virtual std::string cmp(const Register& argument, int constant) const = 0;
    virtual std::string cmp(const MemoryOperand& leftArgument, int constant) const = 0;

    virtual std::string label(std::string name) const = 0;
    virtual std::string jmp(std::string label) const = 0;
    virtual std::string je(std::string label) const = 0;
    virtual std::string jne(std::string label) const = 0;
    virtual std::string jg(std::string label) const = 0; // signed
    virtual std::string jl(std::string label) const = 0; // signed
    virtual std::string jge(std::string label) const = 0; // signed
    virtual std::string jle(std::string label) const = 0; // signed

    virtual std::string syscall() const = 0;
    virtual std::string leave() const = 0;
    virtual std::string ret() const = 0;

    virtual std::string xor_(const Register& operand, const Register& result) const = 0;
    virtual std::string xor_(const MemoryOperand& operand, const Register& result) const = 0;

    virtual std::string or_(const Register& operand, const Register& result) const = 0;
    virtual std::string or_(const MemoryOperand& operand, const Register& result) const = 0;

    virtual std::string and_(const Register& operand, const Register& result) const = 0;
    virtual std::string and_(const MemoryOperand& operand, const Register& result) const = 0;

    virtual std::string shl(const Register& result) const = 0;
    //virtual std::string shl(std::string constant, const Register& result) const = 0;
    virtual std::string shr(const Register& result) const = 0;
    //virtual std::string shr(std::string constant, const Register& result) const = 0;

    virtual std::string add(const Register& operand, const Register& result) const = 0;
    virtual std::string add(const MemoryOperand& operand, const Register& result) const = 0;

    virtual std::string sub(const Register& operand, const Register& result) const = 0;
    virtual std::string sub(const MemoryOperand& operand, const Register& result) const = 0;

    virtual std::string imul(const Register& operand) const = 0;
    virtual std::string imul(const MemoryOperand& operand) const = 0;

    virtual std::string idiv(const Register& operand) const = 0;
    virtual std::string idiv(const MemoryOperand& operand) const = 0;

    virtual std::string inc(const Register& operand) const = 0;
    virtual std::string inc(const MemoryOperand& operand) const = 0;

    virtual std::string dec(const Register& operand) const = 0;
    virtual std::string dec(const MemoryOperand& operand) const = 0;

    virtual std::string neg(const Register& operand) const = 0;
};

} // namespace codegen

#endif // INSTRUCTIONSET_H_
