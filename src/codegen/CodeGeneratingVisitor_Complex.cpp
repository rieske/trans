#include "CodeGeneratingVisitor.h"

#include <cassert>
#include <stdexcept>

#include "Instruction.h"
#include "ValueKind.h"
#include "types/ObjectAbi.h"
#include "types/SysVClassify.h"
#include "types/TypeQuery.h"

namespace codegen {

std::string CodeGeneratingVisitor::addScratchValue(const type::Type& scratchType) {
    assert(currentProcedure_ && "scratch Value outside of a procedure");
    int index = 0;
    auto consider = [&index](const Value& v) {
        const int end = v.getIndex() + type::object_abi::valueWords(v.getSizeInBytes());
        if (end > index) {
            index = end;
        }
    };
    for (const auto& v : currentProcedure_->frame.locals) {
        consider(v);
    }
    for (const auto& v : currentProcedure_->frame.arguments) {
        consider(v);
    }
    const int home = type::object_abi::takeAlignedWords(
            index, scratchType.getAlignment(), type::object_abi::valueWords(scratchType.getSize()));
    const std::string name = "__cs" + std::to_string(convertLabel_++);
    currentProcedure_->frame.locals.push_back(Value {
            name,
            home,
            valueKindFromCType(scratchType),
            scratchType.getSize(),
            type::sysv::classify(scratchType)
    });
    return name;
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

void CodeGeneratingVisitor::emitComplexMulDiv(char op, const std::string& left,
        const std::string& right, const std::string& result, const type::Type& resultType) {
    const type::Type real = type::correspondingReal(resultType);
    const char* helper = complexLibgcc(op, real);
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

void CodeGeneratingVisitor::emitMulDiv(char op, const std::string& left,
        const std::string& right, const std::string& result, const type::Type& resultType) {
    if (type::isComplex(resultType)) {
        emitComplexMulDiv(op, left, right, result, resultType);
        return;
    }
    emitIntegerMulDiv(op, left, right, result, resultType);
}

} // namespace codegen
