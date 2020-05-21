#ifndef INSTRUCTIONS_H_
#define INSTRUCTIONS_H_

#include "codegen/Register.h"
#include "codegen/InstructionSet.h"

namespace codegen {

class Instruction {
public:
    virtual ~Instruction() = default;

    virtual void visit(InstructionSet& instructionSet) const = 0;
};

class Mov: public Instruction {
public:
    Mov(std::string_view source, std::string_view destination);
    virtual ~Mov() = default;

protected:
    std::string source;
    std::string destination;
};

class MovRegReg: public Mov {
public:
    MovRegReg(const Register& source, const Register& destination);
    virtual ~MovRegReg() = default;

    void visit(InstructionSet& instructionSet) const override;
};

    /*Mov(const Register& source, const Register& memoryBase, int memoryOffset);
    Mov(const Register& source, const Register& destination);
    Mov(const Register& memoryBase, int memoryOffset, const Register& destination);
    Mov(std::string_view constant, const Register& memoryBase, int memoryOffset);
    Mov(std::string_view constant, const Register& destination);*/

}

#endif /* INSTRUCTIONS_H_ */
