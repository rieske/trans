#ifndef VAOP_H_
#define VAOP_H_

#include <string>

#include "Quadruple.h"

namespace codegen {

// System V AMD64 va_list operations lowered from __builtin_va_*.
// apPtr / dstPtr / srcPtr are symbols holding a pointer to the 24-byte tag.
// lastAddr (Start) is &last named, or empty for C23 va_start(ap) with gp_offset=0.
class VaOp: public Quadruple {
public:
    enum class Kind {
        Start,
        Arg,
        Copy,
        End
    };

    static VaOp start(std::string apPtr, std::string lastAddr);
    static VaOp arg(std::string apPtr, std::string result, int accessSizeBytes, bool isFloating, bool isSigned);
    static VaOp copy(std::string dstPtr, std::string srcPtr);
    static VaOp end();

    Kind getKind() const { return kind; }
    const std::string& getApPtr() const { return apPtr; }
    const std::string& getLastAddr() const { return lastAddr; }
    const std::string& getResult() const { return result; }
    const std::string& getSrcPtr() const { return srcPtr; }
    int getAccessSizeBytes() const { return accessSizeBytes; }
    bool isFloating() const { return floating; }
    bool isSignedAccess() const { return signedAccess; }

    void generateCode(AssemblyGenerator& generator) const override;

    void collectSymbolRefs(SymbolRefs& refs) const override;

private:
    VaOp(Kind kind);

    void print(std::ostream& stream) const override;

    Kind kind;
    std::string apPtr;
    std::string lastAddr;
    std::string result;
    std::string srcPtr;
    int accessSizeBytes { 8 };
    bool floating { false };
    bool signedAccess { true };
};

} // namespace codegen

#endif // VAOP_H_
