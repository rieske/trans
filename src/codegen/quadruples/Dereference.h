#ifndef DEREFERENCE_H_
#define DEREFERENCE_H_

#include "SingleOperandQuadruple.h"

namespace codegen {

class Dereference: public SingleOperandQuadruple {
public:
    Dereference(std::string operand, std::string lvalue, std::string result, int accessSizeBytes = 8,
            bool isSigned = true);
    virtual ~Dereference() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    std::string getLvalue() const;
    int getAccessSizeBytes() const { return accessSizeBytes; }
    bool isSignedAccess() const { return isSigned; }

    void collectSymbolRefs(SymbolRefs& refs) const override;

private:
    void print(std::ostream& stream) const override;

    std::string lvalue;
    int accessSizeBytes;
    bool isSigned;
};

} // namespace codegen

#endif // DEREFERENCE_H_
