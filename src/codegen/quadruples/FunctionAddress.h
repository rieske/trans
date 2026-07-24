#ifndef FUNCTIONADDRESS_H_
#define FUNCTIONADDRESS_H_

#include "SingleOperandQuadruple.h"

namespace codegen {

// Load the address of a function label into a result temporary.
class FunctionAddress: public SingleOperandQuadruple {
public:
    FunctionAddress(std::string functionName, std::string result);
    virtual ~FunctionAddress() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    void collectSymbolRefs(SymbolRefs& refs) const override;

private:
    void print(std::ostream& stream) const override;
};

} // namespace codegen

#endif // FUNCTIONADDRESS_H_
