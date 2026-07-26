#ifndef INDEXADDRESS_QUADRUPLE_H_
#define INDEXADDRESS_QUADRUPLE_H_

#include <string>
#include "Quadruple.h"
#include "symbols/AddressPlan.h"

namespace codegen {

// result = &base[index] with element stride elementSizeBytes.
// LeaObject: base is the array object (LEA); PointerValue: base holds a pointer.
class IndexAddress: public Quadruple {
public:
    IndexAddress(std::string base, std::string index, int elementSizeBytes, std::string result,
            symbols::AddressBaseMode baseMode = symbols::AddressBaseMode::LeaObject);
    void generateCode(AssemblyGenerator& generator) const override;

    std::string getBase() const { return base; }
    std::string getIndex() const { return index; }
    int getElementSizeBytes() const { return elementSizeBytes; }
    std::string getResult() const { return result; }
    symbols::AddressBaseMode baseMode() const { return baseMode_; }

private:
    void print(std::ostream& stream) const override;
    std::string base;
    std::string index;
    int elementSizeBytes;
    std::string result;
    symbols::AddressBaseMode baseMode_;
};

} // namespace codegen

#endif
