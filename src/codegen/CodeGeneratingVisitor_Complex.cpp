#include "CodeGeneratingVisitor.h"

#include <cassert>
#include <stdexcept>

#include "Instruction.h"
#include "types/TypeQuery.h"

namespace codegen {

int CodeGeneratingVisitor::addScratchValue(const type::Type& scratchType) {
    assert(currentProcedure_ && "scratch Value outside of a procedure");
    return addFrameTemp(module_.strings, *currentProcedure_, scratchType);
}

namespace {

const char* complexLibgcc(char op, const type::Type& real) {
    if (type::isLongDouble(real)) {
        return op == '/' ? "__divxc3" : "__mulxc3";
    }
    if (type::isDouble(real)) {
        return op == '/' ? "__divdc3" : "__muldc3";
    }
    if (type::isFloat(real)) {
        return op == '/' ? "__divsc3" : "__mulsc3";
    }
    throw std::logic_error { "complexLibgcc: corresponding real is not float, double, or long double" };
}

} // namespace

void CodeGeneratingVisitor::emitComplexMulDiv(char op, int left,
        int right, int result, const type::Type& resultType) {
    const type::Type real = type::correspondingReal(resultType);
    const char* helper = complexLibgcc(op, real);
    const int reL = addScratchValue(real);
    const int imL = addScratchValue(real);
    const int reR = addScratchValue(real);
    const int imR = addScratchValue(real);
    emit(ir::assign(left, reL));
    emit(ir::copyPart(left, imL, real.getSize()));
    emit(ir::assign(right, reR));
    emit(ir::copyPart(right, imR, real.getSize()));
    emit(ir::argument(reL));
    emit(ir::argument(imL));
    emit(ir::argument(reR));
    emit(ir::argument(imR));
    emit(ir::call(id(helper)));
    emit(ir::retrieve(result));
}

void CodeGeneratingVisitor::emitMulDiv(char op, int left,
        int right, int result, const type::Type& resultType) {
    if (type::isComplex(resultType)) {
        emitComplexMulDiv(op, left, right, result, resultType);
        return;
    }
    emitIntegerMulDiv(op, left, right, result, resultType);
}

} // namespace codegen
