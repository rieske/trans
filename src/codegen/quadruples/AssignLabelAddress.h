#ifndef ASSIGNLABELADDRESS_H_
#define ASSIGNLABELADDRESS_H_

#include <string>

#include "Quadruple.h"

namespace codegen {

// Address of a pool/data label (not an integer immediate).
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
