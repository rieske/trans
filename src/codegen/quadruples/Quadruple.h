#ifndef QUADRUPLE_H_
#define QUADRUPLE_H_

#include <ostream>

namespace codegen {

class AssemblyGenerator;

class Quadruple {
public:
    virtual ~Quadruple() = default;

    virtual void generateCode(AssemblyGenerator& generator) const = 0;

    virtual bool isLabel() const {
        return false;
    }

    virtual bool transfersControl() const {
        return false;
    }

    friend std::ostream& operator<<(std::ostream& stream, const Quadruple& quadruple) {
        quadruple.print(stream);
        return stream;
    }

protected:
    virtual void print(std::ostream& stream) const = 0;
};

} // namespace codegen

#endif // QUADRUPLE_H_
