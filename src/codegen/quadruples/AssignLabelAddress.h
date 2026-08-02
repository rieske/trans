#ifndef ASSIGNLABELADDRESS_H_
#define ASSIGNLABELADDRESS_H_

#include <string>

#include "Quadruple.h"

namespace codegen {

// Materialize the address of a pool/data label (e.g. string constant L$str1) into result.
// Separate from AssignConstant (integer immediates) so PIE emission cannot confuse the two.
class AssignLabelAddress: public Quadruple {
public:
    AssignLabelAddress(std::string label, std::string result);
    virtual ~AssignLabelAddress() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    std::string getLabel() const;
    std::string getResult() const;

private:
    void print(std::ostream& stream) const override;

    std::string label;
    std::string result;
};

} // namespace codegen

#endif // ASSIGNLABELADDRESS_H_
