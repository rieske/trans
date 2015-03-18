#ifndef ASSEMBLYGENERATOR_H_
#define ASSEMBLYGENERATOR_H_

#include "quadruples/EndProcedure.h"
#include "quadruples/EndScope.h"
#include "quadruples/Input.h"
#include "quadruples/Jump.h"
#include "quadruples/Label.h"
#include "quadruples/Output.h"
#include "quadruples/StartProcedure.h"
#include "quadruples/StartScope.h"
#include "quadruples/ZeroCompare.h"
#include "quadruples/ValueCompare.h"
#include "quadruples/AddressOf.h"
#include "quadruples/Dereference.h"

namespace code_generator {

class AssemblyGenerator {
public:
    AssemblyGenerator();

    void generateCodeFor(const StartScope& startScope);
    void generateCodeFor(const EndScope& endScope);
    void generateCodeFor(const Input& input);
    void generateCodeFor(const Output& output);
    void generateCodeFor(const StartProcedure& startProcedure);
    void generateCodeFor(const EndProcedure& endProcedure);
    void generateCodeFor(const Label& label);
    void generateCodeFor(const Jump& jump);
    void generateCodeFor(const ValueCompare& valueCompare);
    void generateCodeFor(const ZeroCompare& zeroCompare);
    void generateCodeFor(const AddressOf& addressOf);
    void generateCodeFor(const Dereference& dereference);
};

} /* namespace code_generator */

#endif /* ASSEMBLYGENERATOR_H_ */
