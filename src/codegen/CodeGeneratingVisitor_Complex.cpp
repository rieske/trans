#include "CodeGeneratingVisitor.h"
#include "CodeGeneratingVisitorInternal.h"

#include <stdexcept>

#include "Instruction.h"
#include "types/TypeQuery.h"

namespace codegen {

std::string CodeGeneratingVisitor::addScratchValue(const type::Type& scratchType) {
    if (!currentLocals_) {
        throw std::logic_error { "addScratchValue outside of a procedure" };
    }
    const std::string name = "__cs" + std::to_string(convertLabel_++);
    currentLocals_->emplace(name, symbols::ValueEntry {
            name, scratchType, translation_unit::Context { "", 0 }, 0 });
    return name;
}

namespace {

const char* complexLibgcc(ast::OperatorKind kind, const type::Type& real) {
    const bool div = kind == ast::OperatorKind::Div;
    if (type::isLongDouble(real)) {
        return div ? "__divxc3" : "__mulxc3";
    }
    if (type::isDouble(real)) {
        return div ? "__divdc3" : "__muldc3";
    }
    if (type::isFloat(real)) {
        return div ? "__divsc3" : "__mulsc3";
    }
    throw std::logic_error { "complexLibgcc: corresponding real is not float, double, or long double" };
}

} // namespace

void CodeGeneratingVisitor::emitComplexMulDiv(ast::OperatorKind kind, const std::string& left,
        const std::string& right, const std::string& result, const type::Type& resultType) {
    const type::Type real = type::correspondingReal(resultType);
    const char* helper = complexLibgcc(kind, real);
    const std::string reL = addScratchValue(real);
    const std::string imL = addScratchValue(real);
    const std::string reR = addScratchValue(real);
    const std::string imR = addScratchValue(real);
    emit(ir::assign(left, reL));
    emit(ir::copyPart(left, imL, real.getSize()));
    emit(ir::assign(right, reR));
    emit(ir::copyPart(right, imR, real.getSize()));
    emit(ir::argument(reL));
    emit(ir::argument(imL));
    emit(ir::argument(reR));
    emit(ir::argument(imR));
    emit(ir::call(helper));
    emit(ir::retrieve(result));
}

void CodeGeneratingVisitor::emitMulDiv(ast::OperatorKind kind, const std::string& left,
        const std::string& right, const std::string& result, const type::Type& resultType,
        bool unsignedDiv) {
    if (type::isComplex(resultType)) {
        emitComplexMulDiv(kind, left, right, result, resultType);
        return;
    }
    emitIntegerMulDiv(*this, kind, left, right, result, resultType, unsignedDiv);
}

} // namespace codegen
