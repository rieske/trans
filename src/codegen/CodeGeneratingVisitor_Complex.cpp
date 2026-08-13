#include "CodeGeneratingVisitor.h"

#include <cassert>
#include <stdexcept>

#include "Instruction.h"
#include "ValueKind.h"
#include "ast/UnaryExpression.h"
#include "symbols/AddressPlan.h"
#include "types/ObjectAbi.h"
#include "types/SysVClassify.h"
#include "types/TypeQuery.h"
#include "util/FloatingLiteral.h"

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
    const std::string name = "__cs" + std::to_string(convertLabel_++);
    currentProcedure_->frame.locals.push_back(Value {
            name,
            index,
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

void CodeGeneratingVisitor::emitComplexImaginaryConstant(const std::string& resultName,
        const type::Type& complexType, const util::FloatingBits& imag) {
    if (type::isComplexFloat(complexType)) {
        // SysV float complex: one word, imag in the high 32 bits.
        emit(ir::assignConstant(util::hexImmediate(imag.bits << 32), resultName));
        return;
    }
    if (type::isComplexDouble(complexType)) {
        // real word 0, imag word 1.
        emit(ir::assignConstant("0", util::hexImmediate(imag.bits), resultName));
        return;
    }
    if (type::isComplexLongDouble(complexType)) {
        // real {0,0}, imag at correspondingReal offset.
        const type::Type real = type::correspondingReal(complexType);
        const std::string imagPart = addScratchValue(real);
        emitFloatingConstant(imagPart, imag);
        emit(ir::assignConstant("0", "0", resultName));
        const std::string partAddr = addScratchValue(type::pointer(real));
        emit(ir::fieldAddress(resultName, real.getSize(), partAddr,
                symbols::AddressBaseMode::LeaObject));
        emit(ir::lvalueAssign(imagPart, partAddr));
        return;
    }
    throw std::logic_error { "emitComplexImaginaryConstant: unsupported complex type" };
}

bool CodeGeneratingVisitor::emitRealImag(ast::UnaryExpression& expression) {
    const std::string unaryOp = expression.getOperator()->getLexeme();
    if (unaryOp != "__real__" && unaryOp != "__imag__") {
        return false;
    }
    auto* operand = expression.operandSymbol(store_);
    auto* result = expression.getResultSymbol(store_);
    if (!operand || !result) {
        return true;
    }
    const type::Type operandType = expression.getOperandExpression()->getType();
    if (unaryOp == "__imag__" && !type::isComplex(operandType)) {
        emitFloatingConstant(result->getName(),
                util::encodeFloating(0.0L, result->getType().getSize()));
        return true;
    }
    if (unaryOp == "__real__" && !type::isComplex(operandType)) {
        if (operand->getName() != result->getName()) {
            emit(ir::assign(operand->getName(), result->getName()));
        }
        return true;
    }
    const int offset = (unaryOp == "__imag__")
            ? type::correspondingReal(operandType).getSize() : 0;
    if (auto* partAddr = expression.getLvalueSymbol(store_)) {
        auto* baseSym = expression.getOperandExpression()->addressSymbol(store_);
        const auto mode = baseSym->getType().isPointer()
                ? symbols::AddressBaseMode::PointerValue
                : symbols::AddressBaseMode::LeaObject;
        emit(ir::fieldAddress(baseSym->getName(), offset, partAddr->getName(), mode));
        emit(ir::dereference(partAddr->getName(), partAddr->getName(), result->getName()));
    } else if (offset == 0) {
        emit(ir::assign(operand->getName(), result->getName()));
    } else {
        emit(ir::copyPart(operand->getName(), result->getName(), offset));
    }
    return true;
}

} // namespace codegen
