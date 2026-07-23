#ifndef QUADRUPLE_H_
#define QUADRUPLE_H_

#include <ostream>
#include <string>
#include <vector>

namespace codegen {

class AssemblyGenerator;

// Structured IR symbol references for frame packing / liveness (not print text).
// uses: read operands; defs: written results; addressOfBase: object aliased by AddressOf.
struct SymbolRefs {
    std::vector<std::string> uses;
    std::vector<std::string> defs;
    bool isParam { false };
    bool isCall { false };
    // Non-empty only for AddressOf of an object (not FunctionAddress).
    std::string addressOfBase;

    void addUse(const std::string& name) {
        if (!name.empty()) {
            uses.push_back(name);
        }
    }
    void addDef(const std::string& name) {
        if (!name.empty()) {
            defs.push_back(name);
        }
    }
};

class Quadruple {
public:
    virtual ~Quadruple() = default;

    virtual void generateCode(AssemblyGenerator& generator) const = 0;

    // Collect use/def names and call/param/address-of metadata for liveness analysis.
    virtual void collectSymbolRefs(SymbolRefs& /*refs*/) const {}

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
