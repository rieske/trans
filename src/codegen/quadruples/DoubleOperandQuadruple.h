#ifndef DOUBLEOPERANDQUADRUPLE_H_
#define DOUBLEOPERANDQUADRUPLE_H_

#include <string>

#include "Quadruple.h"

namespace codegen {

class DoubleOperandQuadruple: public Quadruple {
public:
    DoubleOperandQuadruple(std::string leftOperandName, std::string rightOperandName, std::string resultName);
    virtual ~DoubleOperandQuadruple() = default;

    std::string getLeftOperandName() const;
    std::string getRightOperandName() const;
    std::string getResultName() const;

private:
    std::string leftOperandName;
    std::string rightOperandName;
    std::string resultName;
};

} // namespace codegen

#endif // DOUBLEOPERANDQUADRUPLE_H_
