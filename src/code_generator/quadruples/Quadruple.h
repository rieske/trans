#ifndef QUADRUPLE_H_
#define QUADRUPLE_H_

namespace code_generator {

class AssemblyGenerator;

class Quadruple {
public:
    virtual ~Quadruple() = default;

    virtual void generateCode(AssemblyGenerator& generator) const = 0;
};

} /* namespace code_generator */

#endif /* QUADRUPLE_H_ */
