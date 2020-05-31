#ifndef STARTPROCEDURE_H_
#define STARTPROCEDURE_H_

#include <iostream>
#include <string>
#include <vector>

#include "../Value.h"
#include "Quadruple.h"

namespace codegen {

class StartProcedure: public Quadruple {
public:
    StartProcedure(std::string name, std::vector<Value> values, std::vector<Value> arguments);
    virtual ~StartProcedure() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    bool isLabel() const override;

    std::string getName() const;
    std::vector<Value> getValues() const;
    std::vector<Value> getArguments() const;

private:
    void print(std::ostream& stream) const override;

    std::string name;
    std::vector<Value> values;
    std::vector<Value> arguments;
};

} /* namespace code_generator */

#endif /* STARTPROCEDURE_H_ */
