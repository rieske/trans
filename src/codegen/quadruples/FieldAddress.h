#ifndef FIELDADDRESS_QUADRUPLE_H_
#define FIELDADDRESS_QUADRUPLE_H_

#include <string>
#include "Quadruple.h"
#include "symbols/AddressPlan.h"

namespace codegen {

// result = address of field using SA AddressBaseMode (LeaObject vs PointerValue).
class FieldAddress: public Quadruple {
public:
    FieldAddress(std::string base, int offsetBytes, std::string result,
            symbols::AddressBaseMode baseMode = symbols::AddressBaseMode::LeaObject);
    void generateCode(AssemblyGenerator& generator) const override;
    std::string getBase() const { return base; }
    int getOffsetBytes() const { return offsetBytes; }
    std::string getResult() const { return result; }
    symbols::AddressBaseMode baseMode() const { return baseMode_; }

private:
    void print(std::ostream& stream) const override;
    std::string base;
    int offsetBytes;
    std::string result;
    symbols::AddressBaseMode baseMode_;
};

} // namespace codegen

#endif
