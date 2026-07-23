#include "VaOp.h"

#include "codegen/AssemblyGenerator.h"

namespace codegen {

VaOp::VaOp(Kind kind) :
        kind { kind }
{
}

VaOp VaOp::start(std::string apPtr, std::string lastAddr) {
    VaOp op(Kind::Start);
    op.apPtr = std::move(apPtr);
    op.lastAddr = std::move(lastAddr);
    return op;
}

VaOp VaOp::arg(std::string apPtr, std::string result, int accessSizeBytes, bool isFloating, bool isSigned) {
    VaOp op(Kind::Arg);
    op.apPtr = std::move(apPtr);
    op.result = std::move(result);
    op.accessSizeBytes = accessSizeBytes;
    op.floating = isFloating;
    op.signedAccess = isSigned;
    return op;
}

VaOp VaOp::copy(std::string dstPtr, std::string srcPtr) {
    VaOp op(Kind::Copy);
    op.apPtr = std::move(dstPtr);
    op.srcPtr = std::move(srcPtr);
    return op;
}

VaOp VaOp::end() {
    return VaOp(Kind::End);
}

void VaOp::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

void VaOp::print(std::ostream& stream) const {
    switch (kind) {
    case Kind::Start:
        stream << "\tva_start(" << apPtr;
        if (!lastAddr.empty()) {
            stream << ", " << lastAddr;
        }
        stream << ")\n";
        break;
    case Kind::Arg:
        stream << "\t" << result << " := va_arg(" << apPtr << ")\n";
        break;
    case Kind::Copy:
        stream << "\tva_copy(" << apPtr << ", " << srcPtr << ")\n";
        break;
    case Kind::End:
        stream << "\tva_end\n";
        break;
    }
}


void VaOp::collectSymbolRefs(SymbolRefs& refs) const {
    refs.addUse(apPtr);
    refs.addUse(lastAddr);
    refs.addDef(result);
    refs.addUse(srcPtr);
}

} // namespace codegen
