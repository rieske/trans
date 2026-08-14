#include "CodeGeneratingVisitor.h"
#include "ast/InitializerListExpression.h"

#include <cassert>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <variant>

#include "symbols/ValueEntry.h"
#include "symbols/LabelEntry.h"
#include "types/ObjectAbiType.h"
#include "types/SysVClassify.h"
#include "types/TypeQuery.h"
#include "util/FloatingLiteral.h"
#include "util/ImmediateFormat.h"
#include "util/IntegerLiteral.h"

#include "Instruction.h"
#include "ValueKind.h"
#include "ast/GnuBuiltinFunctions.h"

namespace {

codegen::Value valueFromSymbol(const symbols::ValueEntry& symbol) {
    return codegen::Value {
            symbol.getName(),
            symbol.getIndex(),
            codegen::valueKindFromCType(symbol.getType()),
            symbol.getType().getSize(),
            type::sysv::classify(symbol.getType())
    };
}

} // namespace

namespace codegen {

CodeGeneratingVisitor::CodeGeneratingVisitor(symbols::AnnotationStore& store) : store_ { store } {
}

CodeGeneratingVisitor::~CodeGeneratingVisitor() {
}

void CodeGeneratingVisitor::emit(Instruction instruction) {
    if (!currentBody_) {
        throw std::logic_error { "CodeGeneratingVisitor: emit outside of a procedure body" };
    }
    currentBody_->push_back(std::move(instruction));
}

void CodeGeneratingVisitor::emitBooleanConvert(const std::string& sourceName,
        const std::string& destName) {
    const std::string one = "__bc" + std::to_string(convertLabel_++) + "t";
    const std::string done = "__bc" + std::to_string(convertLabel_++) + "d";
    emit(ir::zeroCompare(sourceName));
    emit(ir::jump(one, JumpCondition::IF_NOT_EQUAL));
    emit(ir::assignConstant("0", destName));
    emit(ir::jump(done));
    emit(ir::label(one));
    emit(ir::assignConstant("1", destName));
    emit(ir::label(done));
}

void CodeGeneratingVisitor::emitConvert(const std::string& sourceName, const std::string& destName,
        const type::Type& sourceType, const type::Type& destType) {
    if (type::needsBoolConvert(sourceType, destType)) {
        emitBooleanConvert(sourceName, destName);
        return;
    }
    if (type::needsIntegerWiden(sourceType, destType)
            || type::needsIntegerToPointerExtend(sourceType, destType)) {
        emit(ir::widen(sourceName, destName, type::valueIsSigned(sourceType)));
        return;
    }
    emit(ir::assign(sourceName, destName));
}

void CodeGeneratingVisitor::emitIntegerMulDiv(char op, const std::string& left,
        const std::string& right, const std::string& result, const type::Type& resultType) {
    if (type::isIntegral(resultType) && type::object_abi::valueWords(resultType.getSize()) > 1) {
        const char* helper = "__multi3";
        if (op == '/') {
            helper = type::valueIsSigned(resultType) ? "__divti3" : "__udivti3";
        } else if (op == '%') {
            helper = type::valueIsSigned(resultType) ? "__modti3" : "__umodti3";
        }
        emit(ir::argument(left));
        emit(ir::argument(right));
        emit(ir::call(helper));
        emit(ir::retrieve(result));
        return;
    }
    if (op == '*') {
        emit(ir::mul(left, right, result));
    } else if (op == '/') {
        emit(ir::div(left, right, result, type::valueIsSigned(resultType)));
    } else {
        emit(ir::mod(left, right, result, type::valueIsSigned(resultType)));
    }
}

symbols::ValueEntry* CodeGeneratingVisitor::objectHome(ast::Expression& expression) const {
    auto* result = expression.getResultSymbol(store_);
    auto* lv = expression.getLvalueSymbol(store_);
    // Call-arg array decay: Lvalue holds the array object, Result is the pointer temp.
    if (lv && result && lv->getType().isArray() && result->getType().isPointer()) {
        return lv;
    }
    return result;
}

std::string CodeGeneratingVisitor::convertedResultName(ast::Expression& expression) {
    auto* result = expression.getResultSymbol(store_);
    auto* object = objectHome(expression);
    if (object && result && object != result) {
        emit(ir::addressOf(object->getName(), result->getName()));
        return result->getName();
    }
    if (auto* convert = store_.conversion(&expression)) {
        emitConvert(result->getName(), convert->getName(), result->getType(), convert->getType());
        return convert->getName();
    }
    return result->getName();
}

void CodeGeneratingVisitor::emitStructFieldInits(const std::string& objectName,
        const std::vector<symbols::StructFieldInit>& fieldStores) {
    for (const auto& field : fieldStores) {
        emit(ir::fieldAddress(
                objectName, field.offsetBytes, field.addressName,
                symbols::AddressBaseMode::LeaObject));
        if (field.immediate) {
            emit(ir::assignConstant(*field.immediate, field.sourceName));
        } else if (field.zeroInitialize) {
            emit(ir::assignConstant("0", field.sourceName));
        }
        if (field.isBitField()) {
            emitBitFieldInsert(field.addressName, field.sourceName, *field.bitField, field.type);
        } else {
            emit(ir::lvalueAssign(field.sourceName, field.addressName));
        }
    }
}

void CodeGeneratingVisitor::visit(ast::DeclarationSpecifiers&) {
}

void CodeGeneratingVisitor::visit(ast::Declaration& declaration) {
    declaration.visitChildren(*this);
}

void CodeGeneratingVisitor::visit(ast::Declarator& declarator) {
    declarator.visitChildren(*this);
}

void CodeGeneratingVisitor::visit(ast::InitializedDeclarator& declarator) {
    auto* holder = declarator.getHolder(store_);
    // .data init; visiting children would emit assigns with no procedure.
    if (declarator.hasInitializer() && holder && holder->isGlobal()) {
        return;
    }
    declarator.visitChildren(*this);
    if (!declarator.hasInitializer()) {
        return;
    }
    assert(holder && "InitializedDeclarator holder required after successful SA");
    const auto& fieldStores = store_.structFieldInits(&declarator);
    if (!fieldStores.empty()) {
        emitStructFieldInits(holder->getName(), fieldStores);
        return;
    }
    if (declarator.getInitializer()->hasResultSymbol(store_)) {
        emit(ir::assign(
                convertedResultName(*declarator.getInitializer()), holder->getName()));
    }
}

void CodeGeneratingVisitor::visit(ast::ArrayAccess& arrayAccess) {
    arrayAccess.visitLeftOperand(*this);
    arrayAccess.visitRightOperand(*this);
    if (!arrayAccess.getLvalueSymbol(store_) || !arrayAccess.getResultSymbol(store_)) {
        return;
    }
    const auto* indexPlan = store_.addressPlan(&arrayAccess);
    const auto* index = indexPlan ? symbols::get_if<symbols::IndexPlan>(indexPlan) : nullptr;
    // SA always publishes IndexPlan for successful array access analysis.
    assert(index && "IndexPlan required for array codegen");
    ast::Expression& baseExpr = symbols::pickBinaryOperand(
            *arrayAccess.getLeftOperand(), *arrayAccess.getRightOperand(), index->baseOperand);
    ast::Expression& indexExpr = symbols::pickBinaryOperand(
            *arrayAccess.getLeftOperand(), *arrayAccess.getRightOperand(),
            symbols::otherBinaryOperand(index->baseOperand));
    emit(ir::indexAddress(
            baseExpr.getResultSymbol(store_)->getName(),
            indexExpr.getResultSymbol(store_)->getName(),
            index->elementSize,
            arrayAccess.getLvalueSymbol(store_)->getName(),
            index->baseMode));
    if (!arrayAccess.holdsAggregateAddress()) {
        emit(ir::dereference(
                arrayAccess.getLvalueSymbol(store_)->getName(),
                arrayAccess.getLvalueSymbol(store_)->getName(),
                arrayAccess.getResultSymbol(store_)->getName()));
    }
}

void CodeGeneratingVisitor::visit(ast::InitializerListExpression& expression) {
    expression.visitElements(*this);
    // FieldPlanSink names Conversion temps; emit that IR here so stores see filled temps.
    for (const auto& element : expression.getElements()) {
        if (element.value && element.value->hasResultSymbol(store_)) {
            convertedResultName(*element.value);
        }
    }
}

void CodeGeneratingVisitor::visit(ast::MemberAccess& memberAccess) {
    memberAccess.getBase()->accept(*this);
    if (!memberAccess.getLvalueSymbol(store_) || !memberAccess.getResultSymbol(store_)) {
        return;
    }
    const auto* plan = store_.addressPlan(&memberAccess);
    const auto* field = plan ? symbols::get_if<symbols::FieldPlan>(plan) : nullptr;
    assert(field && "FieldPlan required for member access codegen");
    const symbols::ValueEntry* baseSym = memberAccess.getBase()->addressSymbol(store_);
    assert(baseSym && "member base symbol required after successful SA");
    const std::string addrTemp = memberAccess.getLvalueSymbol(store_)->getName();
    const auto baseMode = baseSym->getType().isPointer()
            ? symbols::AddressBaseMode::PointerValue
            : symbols::AddressBaseMode::LeaObject;
    emit(ir::fieldAddress(
            baseSym->getName(),
            field->fieldOffsetBytes,
            addrTemp,
            baseMode));
    if (!memberAccess.holdsAggregateAddress()) {
        const std::string resultName = memberAccess.getResultSymbol(store_)->getName();
        emit(ir::dereference(addrTemp, addrTemp, resultName));
        if (field->isBitField()) {
            emitBitFieldExtract(resultName, resultName, *field->bitField);
        }
    }
}

bool CodeGeneratingVisitor::tryEmitGnuDirectCall(ast::FunctionCall& functionCall,
        const std::string& calleeName) {
    const auto* bswap = ast::findGnuBswapBuiltin(calleeName);
    const auto* ctz = ast::findGnuCtzBuiltin(calleeName);
    if (!bswap && !ctz && !ast::isGnuAllocaBuiltin(calleeName)) {
        return false;
    }
    functionCall.visitArguments(*this);
    const auto& args = functionCall.getArgumentList();
    const std::string arg = convertedResultName(*args[0]);
    const std::string result = functionCall.getResultSymbol(store_)->getName();
    if (bswap) {
        emit(ir::bswap(arg, result, bswap->widthBytes));
    } else if (ctz) {
        emit(ir::ctz(arg, result, ctz->widthBytes));
    } else {
        emit(ir::allocaBytes(arg, result));
    }
    return true;
}

void CodeGeneratingVisitor::visit(ast::FunctionCall& functionCall) {
    long folded;
    if (functionCall.evaluateConstant(folded) && functionCall.hasResultSymbol(store_)) {
        emit(ir::assignConstant(std::to_string(folded),
                functionCall.getResultSymbol(store_)->getName()));
        return;
    }

    const symbols::CallPlan* plan = store_.callPlan(&functionCall);
    if (!plan) {
        // SA error path - no IR.
        functionCall.visitOperand(*this);
        functionCall.visitArguments(*this);
        return;
    }

    if (const auto* direct = symbols::get_if<symbols::DirectCallPlan>(plan)) {
        if (tryEmitGnuDirectCall(functionCall, direct->calleeName)) {
            return;
        }
    }

    std::visit(
            [&](const auto& arm) {
                using T = std::decay_t<decltype(arm)>;
                if constexpr (std::is_same_v<T, symbols::VaStartPlan>
                        || std::is_same_v<T, symbols::VaEndPlan>
                        || std::is_same_v<T, symbols::VaCopyPlan>
                        || std::is_same_v<T, symbols::VaArgPlan>) {
                    functionCall.visitArguments(*this);
                    const auto& args = functionCall.getArgumentList();
                    if constexpr (std::is_same_v<T, symbols::VaStartPlan>) {
                        std::string lastStorage;
                        if (args.size() >= 2) {
                            lastStorage = args[1]->getResultSymbol(store_)->getName();
                        }
                        emit(ir::vaStart(args[0]->getResultSymbol(store_)->getName(),
                                std::move(lastStorage)));
                    } else if constexpr (std::is_same_v<T, symbols::VaEndPlan>) {
                        emit(ir::vaEnd());
                    } else if constexpr (std::is_same_v<T, symbols::VaCopyPlan>) {
                        emit(ir::vaCopy(args[0]->getResultSymbol(store_)->getName(),
                                args[1]->getResultSymbol(store_)->getName()));
                    } else {
                        emit(ir::vaArg(args[0]->getResultSymbol(store_)->getName(),
                                functionCall.getResultSymbol(store_)->getName()));
                    }
                } else {
                    functionCall.visitOperand(*this);
                    functionCall.visitArguments(*this);
                    for (auto& expression : functionCall.getArgumentList()) {
                        emit(ir::argument(convertedResultName(*expression)));
                    }
                    std::string memoryReturnDest;
                    if (functionCall.hasResultSymbol(store_) && !functionCall.getType().isVoid()) {
                        if (type::object_abi::typeNeedsMemoryReturn(functionCall.getType())) {
                            memoryReturnDest = functionCall.getResultSymbol(store_)->getName();
                        }
                    }
                    emit(ir::call(symbols::callCalleeName(*plan), symbols::isIndirectCall(*plan),
                            memoryReturnDest));
                    if (functionCall.hasResultSymbol(store_) && !functionCall.getType().isVoid()) {
                        emit(ir::retrieve(functionCall.getResultSymbol(store_)->getName(),
                                !memoryReturnDest.empty()));
                    }
                }
            },
            *plan);
}

void CodeGeneratingVisitor::visit(ast::IdentifierExpression& identifier) {
    if (identifier.hasStringConstantLabel()) {
        assert(identifier.hasResultSymbol(store_) && "__func__ needs Result temp");
        emit(ir::assignLabelAddress(
                identifier.getStringConstantLabel(), identifier.getResultSymbol(store_)->getName()));
        return;
    }
    if (identifier.hasFoldedConstant()) {
        assert(identifier.hasResultSymbol(store_) && "folded enumerator needs Result temp");
        emit(ir::assignConstant(
                std::to_string(identifier.getFoldedConstant()),
                identifier.getResultSymbol(store_)->getName()));
        return;
    }
    // Function designators: plan holds the label; Result is the address temp.
    if (const auto* plan = store_.addressPlan(&identifier)) {
        if (const auto* d = symbols::get_if<symbols::FunctionDesignatorPlan>(plan)) {
            if (d->functionName) {
                assert(identifier.hasResultSymbol(store_) && "designator Result required for FunctionAddress");
                emit(ir::functionAddress(
                        *d->functionName, identifier.getResultSymbol(store_)->getName()));
                return;
            }
        }
    }
    assert(!identifier.holdsFunctionDesignator()
            && "designator form without FunctionDesignatorPlan on the store");
}

void CodeGeneratingVisitor::visit(ast::ConstantExpression& constant) {
    // Decode to a numeric immediate so suffixes never reach the assembler raw.
    const std::string resultName = constant.getResultSymbol(store_)->getName();
    if (type::isFloating(constant.expressionType())) {
        util::FloatingBits parsed;
        if (!util::floatingLiteralBits(constant.getValue(), parsed)) {
            throw std::runtime_error { "invalid floating constant: " + constant.getValue() };
        }
        emitFloatingConstant(resultName, parsed);
        return;
    }
    util::IntegerLiteral lit;
    if (util::parseIntegerLiteral(constant.getValue(), lit)) {
        const std::string lo = util::wordImmediate(static_cast<unsigned long long>(lit.value));
        if (type::isIntegral(constant.expressionType())
                && type::object_abi::valueWords(constant.expressionType().getSize()) > 1) {
            emit(ir::assignConstant(lo,
                    util::wordImmediate(static_cast<unsigned long long>(lit.value >> 64)),
                    resultName));
        } else {
            emit(ir::assignConstant(lo, resultName));
        }
        return;
    }
    long value;
    if (!constant.evaluateConstant(value)) {
        throw std::runtime_error { "invalid integer constant: " + constant.getValue() };
    }
    emit(ir::assignConstant(util::wordImmediate(static_cast<unsigned long long>(value)), resultName));
}

void CodeGeneratingVisitor::visit(ast::StringLiteralExpression& stringLiteral) {
    emit(ir::assignLabelAddress(
            stringLiteral.getConstantSymbol(), stringLiteral.getResultSymbol(store_)->getName()));
}

namespace {

// Scalar ++/-- steps by 1; pointer ++/-- steps by pointee size in bytes.
int incDecStepBytes(const type::Type& valueType) {
    if (valueType.isPointer()) {
        return type::pointerElementStride(valueType);
    }
    return 1;
}

} // namespace

void CodeGeneratingVisitor::emitFloatingConstant(const std::string& dest, const util::FloatingBits& bits) {
    const std::string lo = util::hexImmediate(bits.bits);
    if (bits.sizeBytes > 8) {
        emit(ir::assignConstant(lo, util::hexImmediate(bits.bitsHi), dest));
    } else {
        emit(ir::assignConstant(lo, dest));
    }
}

void CodeGeneratingVisitor::emitIncDec(const std::string& name, const type::Type& valueType, bool increment) {
    if (type::isFloating(valueType)) {
        const std::string one = addScratchValue(valueType);
        emitFloatingConstant(one, util::floatingOne(valueType.getSize()));
        if (increment) {
            emit(ir::add(name, one, name));
        } else {
            emit(ir::sub(name, one, name));
        }
        return;
    }
    const int step = incDecStepBytes(valueType);
    if (increment) {
        emit(ir::inc(name, step));
    } else {
        emit(ir::dec(name, step));
    }
}

void CodeGeneratingVisitor::visit(ast::PostfixExpression& expression) {
    expression.visitOperand(*this);

    auto* pre = expression.getPreOperationSymbol(store_);
    assert(pre && "Postfix PreOperation required after successful SA");
    auto resultSymbolName = expression.getResultSymbol(store_)->getName();
    auto preOperationSymbol = pre->getName();
    emit(ir::assign(resultSymbolName, preOperationSymbol));

    emitIncDec(resultSymbolName, expression.getResultSymbol(store_)->getType(),
            expression.getOperator()->getLexeme() == "++");

    // Dereference (and similar) lvalues: value lives in a temp; store new value through the pointer.
    if (expression.operandLvalueSymbol(store_)) {
        emitLvalueStore(*expression.getOperandExpression(), resultSymbolName);
    }

    expression.setResultSymbol(store_, *pre);
}

void CodeGeneratingVisitor::visit(ast::PrefixExpression& expression) {
    expression.visitOperand(*this);

    auto resultSymbolName = expression.getResultSymbol(store_)->getName();
    emitIncDec(resultSymbolName, expression.getResultSymbol(store_)->getType(),
            expression.getOperator()->getLexeme() == "++");

    if (expression.operandLvalueSymbol(store_)) {
        emitLvalueStore(*expression.getOperandExpression(), resultSymbolName);
    }
}

void CodeGeneratingVisitor::visit(ast::UnaryExpression& expression) {
    if (expression.getOperator()->getLexeme() == "sizeof") {
        // Operand is unevaluated at runtime; emit the folded size constant.
        emit(ir::assignConstant(
                std::to_string(expression.getSizeofValue()), expression.getResultSymbol(store_)->getName()));
        return;
    }

    expression.visitOperand(*this);

    switch (expression.getOperator()->getLexeme().front()) {
    case '&':
        // &function designator: SA reuses the designator temp (already emitted FunctionAddress).
        if (expression.getOperandExpression()->holdsFunctionDesignator()) {
            break;
        } else if (auto* lvalue = expression.operandLvalueSymbol(store_)) {
            // &a[i] / &*p: address is already computed in the operand's lvalue temp.
            emit(ir::assign(
                    lvalue->getName(), expression.getResultSymbol(store_)->getName()));
        } else {
            emit(ir::addressOf(
                    expression.operandSymbol(store_)->getName(), expression.getResultSymbol(store_)->getName()));
        }
        break;
    case '*':
        if (expression.operandSymbol(store_)->getType().isPointer()) {
            // *fp for pointer-to-function: SA keeps the pointer value (no memory load).
            if (type::isPointerToBareFunction(expression.operandSymbol(store_)->getType())) {
                if (expression.operandSymbol(store_)->getName() != expression.getResultSymbol(store_)->getName()) {
                    emit(ir::assign(
                            expression.operandSymbol(store_)->getName(), expression.getResultSymbol(store_)->getName()));
                }
                break;
            }
            // Already an address (pointer or multi-dim decayed row).
            if (expression.getResultSymbol(store_)->getName() == expression.getLvalueSymbol(store_)->getName()) {
                // Address-only multi-dim *a: just materialize &array into the temp if needed.
                // Result and lvalue share the address temp; operand is the array object.
                if (expression.operandType().isArray()) {
                    emit(ir::addressOf(
                            expression.operandSymbol(store_)->getName(), expression.getLvalueSymbol(store_)->getName()));
                } else {
                    emit(ir::assign(
                            expression.operandSymbol(store_)->getName(), expression.getResultSymbol(store_)->getName()));
                }
            } else {
                emit(ir::dereference(expression.operandSymbol(store_)->getName(),
                        expression.getLvalueSymbol(store_)->getName(), expression.getResultSymbol(store_)->getName()));
            }
        } else if (expression.operandType().isArray()) {
            // True array object: &a then optional load.
            emit(ir::addressOf(
                    expression.operandSymbol(store_)->getName(), expression.getLvalueSymbol(store_)->getName()));
            if (expression.getResultSymbol(store_)->getName() != expression.getLvalueSymbol(store_)->getName()) {
                emit(ir::dereference(
                        expression.getLvalueSymbol(store_)->getName(),
                        expression.getLvalueSymbol(store_)->getName(),
                        expression.getResultSymbol(store_)->getName()));
            }
        } else {
            emit(ir::dereference(expression.operandSymbol(store_)->getName(),
                    expression.getLvalueSymbol(store_)->getName(), expression.getResultSymbol(store_)->getName()));
        }
        break;
    case '+':
        emit(ir::assign(
                convertedResultName(*expression.getOperandExpression()),
                expression.getResultSymbol(store_)->getName()));
        break;
    case '-':
        emit(ir::unaryMinus(convertedResultName(*expression.getOperandExpression()),
                expression.getResultSymbol(store_)->getName()));
        break;
    case '~':
        emit(ir::unaryNot(convertedResultName(*expression.getOperandExpression()),
                expression.getResultSymbol(store_)->getName()));
        break;
    case '!':
        emit(ir::zeroCompare(expression.operandSymbol(store_)->getName()));
        emit(ir::jump(expression.getTruthyLabel(store_)->getName(), JumpCondition::IF_EQUAL));
        emit(ir::assignConstant("0", expression.getResultSymbol(store_)->getName()));
        emit(ir::jump(expression.getFalsyLabel(store_)->getName()));
        emit(ir::label(expression.getTruthyLabel(store_)->getName()));
        emit(ir::assignConstant("1", expression.getResultSymbol(store_)->getName()));
        emit(ir::label(expression.getFalsyLabel(store_)->getName()));
        break;
    default:
        throw std::runtime_error { "Unidentified unary operator: " + expression.getOperator()->getLexeme() };
    }
}

void CodeGeneratingVisitor::visit(ast::StatementExpression& expression) {
    expression.body().accept(*this);
}

void CodeGeneratingVisitor::visit(ast::GenericSelection& expression) {
    if (!expression.hasSelected()) {
        return;
    }
    expression.selectedExpression().accept(*this);
}

void CodeGeneratingVisitor::visit(ast::CompoundLiteral& expression) {
    expression.initializer().accept(*this);
    auto* object = objectHome(expression);
    if (!object) {
        return;
    }
    const auto& fieldStores = store_.structFieldInits(&expression);
    if (!fieldStores.empty()) {
        emitStructFieldInits(object->getName(), fieldStores);
        return;
    }
    if (expression.initializer().hasResultSymbol(store_)) {
        emit(ir::assign(convertedResultName(expression.initializer()), object->getName()));
    }
}

void CodeGeneratingVisitor::visit(ast::TypeCast& expression) {
    expression.visitOperand(*this);
    auto* source = expression.operandSymbol(store_);
    auto* dest = expression.getResultSymbol(store_);
    // Only true array objects need AddressOf. Multi-dim rows already hold a decayed pointer
    // in the result symbol while expression type may still be array.
    if (source->getType().isArray()) {
        emit(ir::addressOf(source->getName(), dest->getName()));
    } else {
        emitConvert(source->getName(), dest->getName(), source->getType(), dest->getType());
    }
}

void CodeGeneratingVisitor::visit(ast::ArithmeticExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    const auto* leftSym = expression.leftOperandSymbol(store_);
    const auto* rightSym = expression.rightOperandSymbol(store_);
    const auto* resultSym = expression.getResultSymbol(store_);
    if (!leftSym || !rightSym || !resultSym) {
        return;
    }
    const type::Type leftType = leftSym->getType();
    const type::Type rightType = rightSym->getType();
    const char op = expression.getOperator()->getLexeme().front();
    const std::string leftName = convertedResultName(*expression.getLeftOperand());
    const std::string rightName = convertedResultName(*expression.getRightOperand());
    const std::string resultName = resultSym->getName();

    if (op == '+' || op == '-') {
        emitAdditive(op, leftType, rightType, leftName, rightName, resultName);
        return;
    }
    if (op == '*' || op == '/' || op == '%') {
        emitMulDiv(op, leftName, rightName, resultName, resultSym->getType());
        return;
    }
    throw std::runtime_error { "unidentified arithmetic operator: " + expression.getOperator()->getLexeme() };
}

void CodeGeneratingVisitor::emitAdditive(char op, const type::Type& leftType, const type::Type& rightType,
        const std::string& leftName, const std::string& rightName, const std::string& resultName) {
    const type::PointerArithmeticInfo ptrArith = type::classifyPointerArithmetic(leftType, rightType, op);
    switch (ptrArith.form) {
    case type::PointerArithmeticForm::None:
        break;
    case type::PointerArithmeticForm::PtrPlusInt:
    case type::PointerArithmeticForm::IntPlusPtr:
    case type::PointerArithmeticForm::PtrMinusInt: {
        const bool intLeft = ptrArith.form == type::PointerArithmeticForm::IntPlusPtr;
        const bool subtract = ptrArith.form == type::PointerArithmeticForm::PtrMinusInt;
        emit(ir::pointerOffset(
                intLeft ? rightName : leftName,
                intLeft ? leftName : rightName,
                ptrArith.strideBytes, resultName, subtract));
        return;
    }
    case type::PointerArithmeticForm::PtrMinusPtr:
        emit(ir::pointerDiff(leftName, rightName, ptrArith.strideBytes, resultName));
        return;
    case type::PointerArithmeticForm::Invalid:
        throw std::logic_error("pointer arithmetic Invalid should not reach codegen");
    }
    if (op == '+') {
        emit(ir::add(leftName, rightName, resultName));
        return;
    }
    if (op == '-') {
        emit(ir::sub(leftName, rightName, resultName));
        return;
    }
    throw std::runtime_error { "unidentified additive operator" };
}

void CodeGeneratingVisitor::visit(ast::ShiftExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    const std::string leftName = convertedResultName(*expression.getLeftOperand());
    const std::string rightName = convertedResultName(*expression.getRightOperand());
    const std::string resultName = expression.getResultSymbol(store_)->getName();
    switch (expression.getOperator()->getLexeme().front()) {
    case '<':   // <<
        emit(ir::shl(leftName, rightName, resultName));
        break;
    case '>':   // >>
        emit(ir::shr(leftName, rightName, resultName,
                type::valueIsSigned(expression.getResultSymbol(store_)->getType())));
        break;
    default:
        throw std::runtime_error { "unidentified shift operator!" };
    }
}

void CodeGeneratingVisitor::visit(ast::ComparisonExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    const auto* leftSym = expression.leftOperandSymbol(store_);
    const auto* rightSym = expression.rightOperandSymbol(store_);
    const type::Type uac = type::usualArithmeticResult(leftSym->getType(), rightSym->getType());
    const bool signedRel = type::valueIsSigned(uac);
    emit(ir::valueCompare(
            convertedResultName(*expression.getLeftOperand()),
            convertedResultName(*expression.getRightOperand()),
            signedRel));

    auto truthyLabel = expression.getTruthyLabel(store_)->getName();
    if (expression.getOperator()->getLexeme() == ">") {
        emit(ir::jump(truthyLabel, JumpCondition::IF_ABOVE, signedRel));
    } else if (expression.getOperator()->getLexeme() == "<") {
        emit(ir::jump(truthyLabel, JumpCondition::IF_BELOW, signedRel));
    } else if (expression.getOperator()->getLexeme() == "<=") {
        emit(ir::jump(truthyLabel, JumpCondition::IF_BELOW_OR_EQUAL, signedRel));
    } else if (expression.getOperator()->getLexeme() == ">=") {
        emit(ir::jump(truthyLabel, JumpCondition::IF_ABOVE_OR_EQUAL, signedRel));
    } else if (expression.getOperator()->getLexeme() == "==") {
        emit(ir::jump(truthyLabel, JumpCondition::IF_EQUAL));
    } else if (expression.getOperator()->getLexeme() == "!=") {
        emit(ir::jump(truthyLabel, JumpCondition::IF_NOT_EQUAL));
    } else {
        throw std::runtime_error { "unidentified ml_op operator!\n" };
    }

    emit(ir::assignConstant("0", expression.getResultSymbol(store_)->getName()));
    emit(ir::jump(expression.getFalsyLabel(store_)->getName()));
    emit(ir::label(truthyLabel));
    emit(ir::assignConstant("1", expression.getResultSymbol(store_)->getName()));
    emit(ir::label(expression.getFalsyLabel(store_)->getName()));
}

void CodeGeneratingVisitor::visit(ast::BitwiseExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    const std::string leftName = convertedResultName(*expression.getLeftOperand());
    const std::string rightName = convertedResultName(*expression.getRightOperand());
    const std::string resultName = expression.getResultSymbol(store_)->getName();
    switch (expression.getOperator()->getLexeme().front()) {
    case '&':
        emit(ir::andOp(leftName, rightName, resultName));
        break;
    case '|':
        emit(ir::orOp(leftName, rightName, resultName));
        break;
    case '^':
        emit(ir::xorOp(leftName, rightName, resultName));
        break;
    default:
        throw std::runtime_error { "no semantic actions defined for bitwise operator: " + expression.getOperator()->getLexeme() };
    }
}

void CodeGeneratingVisitor::visit(ast::LogicalAndExpression& expression) {
    expression.visitLeftOperand(*this);

    emit(ir::assignConstant("0", expression.getResultSymbol(store_)->getName()));
    emit(ir::zeroCompare(expression.leftOperandSymbol(store_)->getName()));
    emit(ir::jump(expression.getExitLabel(store_)->getName(), JumpCondition::IF_EQUAL));

    expression.visitRightOperand(*this);

    emit(ir::zeroCompare(expression.rightOperandSymbol(store_)->getName()));
    emit(ir::jump(expression.getExitLabel(store_)->getName(), JumpCondition::IF_EQUAL));
    emit(ir::assignConstant("1", expression.getResultSymbol(store_)->getName()));

    emit(ir::label(expression.getExitLabel(store_)->getName()));
}

void CodeGeneratingVisitor::visit(ast::LogicalOrExpression& expression) {
    expression.visitLeftOperand(*this);

    emit(ir::assignConstant("1", expression.getResultSymbol(store_)->getName()));
    emit(ir::zeroCompare(expression.leftOperandSymbol(store_)->getName()));
    emit(ir::jump(expression.getExitLabel(store_)->getName(), JumpCondition::IF_NOT_EQUAL));

    expression.visitRightOperand(*this);

    emit(ir::zeroCompare(expression.rightOperandSymbol(store_)->getName()));
    emit(ir::jump(expression.getExitLabel(store_)->getName(), JumpCondition::IF_NOT_EQUAL));
    emit(ir::assignConstant("0", expression.getResultSymbol(store_)->getName()));

    emit(ir::label(expression.getExitLabel(store_)->getName()));
}

void CodeGeneratingVisitor::visit(ast::ConditionalExpression& expression) {
    expression.visitCondition(*this);
    emit(ir::zeroCompare(expression.conditionSymbol(store_)->getName()));
    emit(ir::jump(expression.getFalsyLabel(store_)->getName(), JumpCondition::IF_EQUAL));

    expression.visitTrueExpression(*this);
    emit(ir::assign(
            convertedResultName(*expression.getTrueExpression()),
            expression.getResultSymbol(store_)->getName()));
    emit(ir::jump(expression.getExitLabel(store_)->getName()));

    emit(ir::label(expression.getFalsyLabel(store_)->getName()));
    expression.visitFalseExpression(*this);
    emit(ir::assign(
            convertedResultName(*expression.getFalseExpression()),
            expression.getResultSymbol(store_)->getName()));

    emit(ir::label(expression.getExitLabel(store_)->getName()));
}

void CodeGeneratingVisitor::visit(ast::AssignmentExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    auto assignmentOperator = expression.getOperator();
    auto resultName = expression.getResultSymbol(store_)->getName();
    const std::string rightName = convertedResultName(*expression.getRightOperand());
    if (assignmentOperator->getLexeme() == "+=" || assignmentOperator->getLexeme() == "-=") {
        const type::Type leftType = expression.getResultSymbol(store_)->getType();
        const type::Type rightType = expression.rightOperandSymbol(store_)->getType();
        emitAdditive(assignmentOperator->getLexeme().front(), leftType, rightType,
                resultName, rightName, resultName);
    }
    else if (assignmentOperator->getLexeme() == "*="
            || assignmentOperator->getLexeme() == "/="
            || assignmentOperator->getLexeme() == "%=") {
        emitMulDiv(assignmentOperator->getLexeme().front(), resultName, rightName, resultName,
                expression.getResultSymbol(store_)->getType());
    }
    else if (assignmentOperator->getLexeme() == "&=")
        emit(ir::andOp(resultName, rightName, resultName));
    else if (assignmentOperator->getLexeme() == "^=")
        emit(ir::xorOp(resultName, rightName, resultName));
    else if (assignmentOperator->getLexeme() == "|=")
        emit(ir::orOp(resultName, rightName, resultName));
    else if (assignmentOperator->getLexeme() == "<<=") {
        emit(ir::shl(resultName, rightName, resultName));
    } else if (assignmentOperator->getLexeme() == ">>=") {
        emit(ir::shr(resultName, rightName, resultName,
                type::valueIsSigned(expression.getResultSymbol(store_)->getType())));
    } else if (assignmentOperator->getLexeme() == "=") {
        if (expression.leftOperandLvalueSymbol(store_)) {
            emit(ir::assign(rightName, resultName));
            emitLvalueStore(*expression.getLeftOperand(), resultName);
        } else {
            emit(ir::assign(rightName, resultName));
        }
        return;
    } else {
        throw std::runtime_error { "unidentified assignment operator: " + assignmentOperator->getLexeme() };
    }

    if (expression.leftOperandLvalueSymbol(store_)) {
        emitLvalueStore(*expression.getLeftOperand(), resultName);
    }
}

void CodeGeneratingVisitor::visit(ast::ExpressionList& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);
}

void CodeGeneratingVisitor::visit(ast::Operator&) {
}

void CodeGeneratingVisitor::visit(ast::Pointer&) {
}

void CodeGeneratingVisitor::visit(ast::Identifier&) {
}

void CodeGeneratingVisitor::visit(ast::FunctionDeclarator& declarator) {
    declarator.visitFormalArguments(*this);
}

void CodeGeneratingVisitor::visit(ast::ArrayDeclarator& declaration) {
    // Size is folded in semantic analysis; visiting the bound would emit into no procedure
    // for file-scope prototypes such as `char[20]`.
    (void)declaration;
}

void CodeGeneratingVisitor::visit(ast::FormalArgument& parameter) {
    parameter.visitDeclarator(*this);
}

void CodeGeneratingVisitor::visit(ast::FunctionDefinition& function) {
    // Semantic analysis skips setSymbol when the definition is invalid (e.g. name conflicts).
    if (!function.hasSymbol()) {
        return;
    }

    function.visitDeclarator(*this);

    std::vector<Value> values;
    for (auto& valueSymbol : function.getLocalVariables()) {
        values.push_back(valueFromSymbol(valueSymbol.second));
    }
    std::vector<Value> arguments;
    for (auto& argumentSymbol : function.getArguments()) {
        arguments.push_back(valueFromSymbol(argumentSymbol));
    }
    Procedure procedure;
    procedure.name = function.getSymbol()->getName();
    procedure.frame.locals = std::move(values);
    procedure.frame.arguments = std::move(arguments);
    const bool variadic = function.getSymbol()->getType().isVariadic();
    procedure.memoryReturn = type::object_abi::typeNeedsMemoryReturn(
            function.getSymbol()->returnType());
    procedure.variadic = variadic;
    procedure.exported = !function.getSymbol()->hasInternalLinkage();

    std::vector<Instruction>* previousBody = currentBody_;
    Procedure* previousProcedure = currentProcedure_;
    currentProcedure_ = &procedure;
    currentBody_ = &procedure.body;
    function.visitBody(*this);
    currentProcedure_ = previousProcedure;
    currentBody_ = previousBody;

    module_.procedures.push_back(std::move(procedure));
}

void CodeGeneratingVisitor::visit(ast::Block& block) {
    block.visitChildren(*this);
}

IntermediateRepresentation CodeGeneratingVisitor::takeIr() {
    return std::move(module_);
}

} // namespace codegen

