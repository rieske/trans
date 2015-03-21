#ifndef DOUBLEOPERANDQUADRUPLE_H_
#define DOUBLEOPERANDQUADRUPLE_H_

#include <string>

#include "Quadruple.h"

namespace code_generator {

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

} /* namespace code_generator */

#endif /* DOUBLEOPERANDQUADRUPLE_H_ */
