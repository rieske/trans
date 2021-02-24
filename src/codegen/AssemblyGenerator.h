#ifndef ASSEMBLYGENERATOR_H_
#define ASSEMBLYGENERATOR_H_

#include <memory>

#include "StackMachine.h"

#include "quadruples/EndProcedure.h"
#include "quadruples/Input.h"
#include "quadruples/Jump.h"
#include "quadruples/Label.h"
#include "quadruples/StartProcedure.h"
#include "quadruples/ZeroCompare.h"
#include "quadruples/ValueCompare.h"
#include "quadruples/AddressOf.h"
#include "quadruples/Dereference.h"
#include "quadruples/UnaryMinus.h"
#include "quadruples/Assign.h"
#include "quadruples/AssignConstant.h"
#include "quadruples/LvalueAssign.h"
#include "quadruples/Argument.h"
#include "quadruples/Call.h"
#include "quadruples/Return.h"
#include "quadruples/VoidReturn.h"
#include "quadruples/Retrieve.h"
#include "quadruples/Xor.h"
#include "quadruples/Or.h"
#include "quadruples/And.h"
#include "quadruples/Add.h"
#include "quadruples/Sub.h"
#include "quadruples/Mul.h"
#include "quadruples/Div.h"
#include "quadruples/Mod.h"
#include "quadruples/Inc.h"
#include "quadruples/Dec.h"

namespace codegen {

class AssemblyGenerator {
public:
    AssemblyGenerator(std::unique_ptr<StackMachine> stackMachine);

    void generateAssemblyCode(std::vector<std::unique_ptr<Quadruple>> quadruples, std::map<std::string, std::string> constants);

    void generateCodeFor(const Input& input);
    void generateCodeFor(const StartProcedure& startProcedure);
    void generateCodeFor(const EndProcedure& endProcedure);
    void generateCodeFor(const Label& label);
    void generateCodeFor(const Jump& jump);
    void generateCodeFor(const ValueCompare& valueCompare);
    void generateCodeFor(const ZeroCompare& zeroCompare);
    void generateCodeFor(const AddressOf& addressOf);
    void generateCodeFor(const Dereference& dereference);
    void generateCodeFor(const UnaryMinus& unaryMinus);
    void generateCodeFor(const Assign& assign);
    void generateCodeFor(const AssignConstant& assignConstant);
    void generateCodeFor(const LvalueAssign& lvalueAssign);
    void generateCodeFor(const Argument& argument);
    void generateCodeFor(const Call& call);
    void generateCodeFor(const Return& returnCommand);
    void generateCodeFor(const VoidReturn& returnCommand);
    void generateCodeFor(const Retrieve& retrieve);
    void generateCodeFor(const Xor& xorCommand);
    void generateCodeFor(const Or& orCommand);
    void generateCodeFor(const And& andCommand);
    void generateCodeFor(const Add& add);
    void generateCodeFor(const Sub& sub);
    void generateCodeFor(const Mul& mul);
    void generateCodeFor(const Div& div);
    void generateCodeFor(const Mod& mod);
    void generateCodeFor(const Inc& inc);
    void generateCodeFor(const Dec& dec);

private:
    std::unique_ptr<StackMachine> stackMachine;
};

} // namespace codegen

#endif // ASSEMBLYGENERATOR_H_
