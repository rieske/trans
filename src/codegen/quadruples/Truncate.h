#ifndef TRUNCATE_H_
#define TRUNCATE_H_

#include <string>

#include "Quadruple.h"

namespace codegen {

// Narrow an integral value in place: zero- or sign-extend to 64-bit width.
// sizeBytes is the C type width (1 or 4); isSigned selects movsx vs movzx.
class Truncate: public Quadruple {
public:
    Truncate(std::string operand, int sizeBytes, bool isSigned);
    virtual ~Truncate() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    std::string getOperandName() const;
    int getSizeBytes() const { return sizeBytes; }
    bool isSigned() const { return signedExtend; }

    void collectSymbolRefs(SymbolRefs& refs) const override;

private:
    void print(std::ostream& stream) const override;

    std::string operandName;
    int sizeBytes;
    bool signedExtend;
};

} // namespace codegen

#endif // TRUNCATE_H_
