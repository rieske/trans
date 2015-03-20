#ifndef INSTRUCTIONSET_H_
#define INSTRUCTIONSET_H_

#include <string>

#include "Register.h"

namespace code_generator {

class InstructionSet {
public:
    InstructionSet();
    virtual ~InstructionSet() = default;

    std::string mainProcedure() const;
    std::string call(std::string procedureName) const;

    std::string push(const Register& reg) const;
    std::string pop(const Register& reg) const;

    std::string add(const Register& reg, int constant) const;
    std::string sub(const Register& reg, int constant) const;

    std::string mov(const Register& from, const Register& memoryBase, int memoryOffset) const;
    std::string mov(const Register& from, const Register& to) const;
    std::string mov(const Register& memoryBase, int memoryOffset, const Register& to) const;

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
};

} /* namespace code_generator */

#endif /* INSTRUCTIONSET_H_ */
