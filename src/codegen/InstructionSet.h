#ifndef INSTRUCTIONSET_H_
#define INSTRUCTIONSET_H_

#include <string>
#include <map>

namespace codegen {

class Register;

class InstructionSet {
public:
    virtual ~InstructionSet() = default;

    virtual std::string preamble(std::map<std::string, std::string> constants) const = 0;

    virtual std::string call(std::string procedureName) const = 0;

    virtual std::string push(const Register& reg) const = 0;
    virtual std::string pop(const Register& reg) const = 0;

    virtual std::string lea(const Register& base, int offset, const Register& target) const = 0;

    virtual std::string add(const Register& reg, int constant) const = 0;
    virtual std::string sub(const Register& reg, int constant) const = 0;

    virtual std::string not_(const Register& reg) const = 0;

    virtual std::string mov(const Register& from, const Register& memoryBase, int memoryOffset) const = 0;
    virtual std::string mov(const Register& from, const Register& to) const = 0;
    virtual std::string mov(const Register& memoryBase, int memoryOffset, const Register& to) const = 0;
    virtual std::string mov(std::string constant, const Register& memoryBase, int memoryOffset) const = 0;
    virtual std::string mov(std::string constant, const Register& to) const = 0;

    virtual std::string cmp(const Register& leftArgument, const Register& memoryBase, int memoryOffset) const = 0;
    virtual std::string cmp(const Register& leftArgument, const Register& rightArgument) const = 0;
    virtual std::string cmp(const Register& memoryBase, int memoryOffset, const Register& rightArgument) const = 0;
    virtual std::string cmp(const Register& argument, int constant) const = 0;
    virtual std::string cmp(const Register& memoryBase, int memoryOffset, int constant) const = 0;

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
    virtual std::string xor_(const Register& operandBase, int operandOffset, const Register& result) const = 0;

    virtual std::string or_(const Register& operand, const Register& result) const = 0;
    virtual std::string or_(const Register& operandBase, int operandOffset, const Register& result) const = 0;

    virtual std::string and_(const Register& operand, const Register& result) const = 0;
    virtual std::string and_(const Register& operandBase, int operandOffset, const Register& result) const = 0;

    virtual std::string add(const Register& operand, const Register& result) const = 0;
    virtual std::string add(const Register& operandBase, int operandOffset, const Register& result) const = 0;

    virtual std::string sub(const Register& operand, const Register& result) const = 0;
    virtual std::string sub(const Register& operandBase, int operandOffset, const Register& result) const = 0;

    virtual std::string imul(const Register& operand) const = 0;
    virtual std::string imul(const Register& operandBase, int operandOffset) const = 0;

    virtual std::string idiv(const Register& operand) const = 0;
    virtual std::string idiv(const Register& operandBase, int operandOffset) const = 0;

    virtual std::string inc(const Register& operand) const = 0;
    virtual std::string inc(const Register& operandBase, int operandOffset) const = 0;

    virtual std::string dec(const Register& operand) const = 0;
    virtual std::string dec(const Register& operandBase, int operandOffset) const = 0;

    virtual std::string neg(const Register& operand) const = 0;
};

} // namespace codegen

#endif // INSTRUCTIONSET_H_
