#include "CodeGeneratingVisitor.h"
#include "ast/InitializerListExpression.h"

#include <cassert>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <variant>

#include "symbols/ValueEntry.h"
#include "types/IntegerConstant.h"
#include "types/ObjectAbiType.h"
#include "types/SysVClassify.h"
#include "types/TypeQuery.h"
#include "util/FloatingLiteral.h"
#include "util/ImmediateFormat.h"

#include "ValueKind.h"
#include "ast/GnuBuiltinFunctions.h"

namespace {

codegen::Value valueFromSymbol(codegen::IrStringTable& strings, const symbols::ValueEntry& symbol) {
    const type::Type& objectType = symbol.getType();
    const type::Type homeType = type::hasRuntimeSize(objectType)
            ? type::pointer(objectType)
            : objectType;
    codegen::Value value {
            strings.intern(symbol.getName()),
            symbol.getIndex(),
            codegen::valueKindFromCType(homeType),
            homeType.getSize(),
            type::sysv::classify(homeType)
    };
    if (symbol.isExpressionTemp()) {
        value.markExpressionTemp();
    }
    return value;
}

} // namespace

namespace codegen {

CodeGeneratingVisitor::CodeGeneratingVisitor(symbols::AnnotationStore& store,
        const ast::VlaExpressionTable* vlas) : store_ { store }, vlas_ { vlas } {
}

CodeGeneratingVisitor::~CodeGeneratingVisitor() {
}

void CodeGeneratingVisitor::emit(Instruction instruction) {
    if (!currentBody_) {
        throw std::logic_error { "CodeGeneratingVisitor: emit outside of a procedure body" };
    }
    currentBody_->push_back(std::move(instruction));
}

int CodeGeneratingVisitor::id(std::string_view name) {
    return module_.strings.intern(name);
}

int CodeGeneratingVisitor::id(const symbols::ValueEntry& symbol) {
    return module_.strings.intern(symbol.getName());
}

int CodeGeneratingVisitor::id(const symbols::LabelEntry& label) {
    return module_.strings.intern(label.getName());
}

void CodeGeneratingVisitor::emitBooleanConvert(int source, int dest) {
    const int one = id("__bc" + std::to_string(convertLabel_++) + "t");
    const int done = id("__bc" + std::to_string(convertLabel_++) + "d");
    emit(ir::zeroCompare(source));
    emit(ir::jump(one, JumpCondition::IF_NOT_EQUAL));
    emit(ir::assignConstant(id("0"), dest));
    emit(ir::jump(done));
    emit(ir::label(one));
    emit(ir::assignConstant(id("1"), dest));
    emit(ir::label(done));
}

void CodeGeneratingVisitor::emitConvert(int source, int dest,
        const type::Type& sourceType, const type::Type& destType) {
    if (type::needsBoolConvert(sourceType, destType)) {
        emitBooleanConvert(source, dest);
        return;
    }
    if (type::needsIntegerWiden(sourceType, destType)
            || type::needsIntegerToPointerExtend(sourceType, destType)) {
        emit(ir::widen(source, dest, type::valueIsSigned(sourceType)));
        return;
    }
    emit(ir::assign(source, dest));
}

void CodeGeneratingVisitor::emitIntegerMulDiv(char op, int left,
        int right, int result, const type::Type& resultType) {
    if (type::isIntegral(resultType) && type::object_abi::valueWords(resultType.getSize()) > 1) {
        const char* helper = "__multi3";
        if (op == '/') {
            helper = type::valueIsSigned(resultType) ? "__divti3" : "__udivti3";
        } else if (op == '%') {
            helper = type::valueIsSigned(resultType) ? "__modti3" : "__umodti3";
        }
        emit(ir::argument(left));
        emit(ir::argument(right));
        emit(ir::call(id(helper)));
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

int CodeGeneratingVisitor::convertedResult(ast::Expression& expression) {
    auto* result = expression.getResultSymbol(store_);
    auto* object = objectHome(expression);
    if (object && result && object != result) {
        emitArrayObjectAddress(*object, id(*result));
        return id(*result);
    }
    if (auto* convert = store_.conversion(&expression)) {
        emitConvert(id(*result), id(*convert), result->getType(), convert->getType());
        return id(*convert);
    }
    return id(*result);
}

void CodeGeneratingVisitor::emitStructFieldInits(int object,
        const std::vector<symbols::StructFieldInit>& fieldStores) {
    for (const auto& field : fieldStores) {
        emit(ir::fieldAddress(
                object, field.offsetBytes, id(field.addressName),
                symbols::AddressBaseMode::LeaObject));
        if (field.immediate) {
            emit(ir::assignConstant(id(*field.immediate), id(field.sourceName)));
        } else if (field.zeroInitialize) {
            emit(ir::assignConstant(id("0"), id(field.sourceName)));
        }
        if (field.isBitField()) {
            emitBitFieldInsert(id(field.addressName), id(field.sourceName), *field.bitField, field.type);
        } else {
            emit(ir::lvalueAssign(id(field.sourceName), id(field.addressName)));
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
    if (holder && !holder->isGlobal() && type::hasComputableRuntimeSize(holder->getType())) {
        const int sizeName = addScratchValue(type::signedInteger());
        emitSizeofProduct(holder->getType(), sizeName);
        emit(ir::allocaBytes(sizeName, id(*holder)));
    }
    if (!declarator.hasInitializer()) {
        return;
    }
    assert(holder && "InitializedDeclarator holder required after successful SA");
    const auto& fieldStores = store_.structFieldInits(&declarator);
    if (!fieldStores.empty()) {
        emitStructFieldInits(id(*holder), fieldStores);
        return;
    }
    if (declarator.getInitializer()->hasResultSymbol(store_)) {
        emit(ir::assign(
                convertedResult(*declarator.getInitializer()), id(*holder)));
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
    const ScaledIndex scaled = scaleIndex(
            index->elementType,
            id(*indexExpr.getResultSymbol(store_)),
            index->elementSize);
    emit(ir::indexAddress(
            id(*baseExpr.getResultSymbol(store_)),
            scaled.name,
            scaled.strideBytes,
            id(*arrayAccess.getLvalueSymbol(store_)),
            index->baseMode));
    if (!arrayAccess.holdsAggregateAddress()) {
        const int addr = id(*arrayAccess.getLvalueSymbol(store_));
        emit(ir::dereference(addr, addr, id(*arrayAccess.getResultSymbol(store_))));
    }
}

void CodeGeneratingVisitor::visit(ast::InitializerListExpression& expression) {
    expression.visitElements(*this);
    // FieldPlanSink names Conversion temps; emit that IR here so stores see filled temps.
    for (const auto& element : expression.getElements()) {
        if (element.value && element.value->hasResultSymbol(store_)) {
            convertedResult(*element.value);
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
    const int addrTemp = id(*memberAccess.getLvalueSymbol(store_));
    const auto baseMode = baseSym->getType().isPointer()
            ? symbols::AddressBaseMode::PointerValue
            : symbols::AddressBaseMode::LeaObject;
    emit(ir::fieldAddress(
            id(*baseSym),
            field->fieldOffsetBytes,
            addrTemp,
            baseMode));
    if (!memberAccess.holdsAggregateAddress()) {
        const int resultName = id(*memberAccess.getResultSymbol(store_));
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
    const int arg = convertedResult(*args[0]);
    const int result = id(*functionCall.getResultSymbol(store_));
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
    type::IntegerConstant folded;
    if (functionCall.evaluateConstant(folded) && functionCall.hasResultSymbol(store_)) {
        emitIntegerConstant(folded, id(*functionCall.getResultSymbol(store_)));
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
                        int lastStorage = kNoSymbol;
                        if (args.size() >= 2) {
                            lastStorage = id(*args[1]->getResultSymbol(store_));
                        }
                        emit(ir::vaStart(id(*args[0]->getResultSymbol(store_)), lastStorage));
                    } else if constexpr (std::is_same_v<T, symbols::VaEndPlan>) {
                        emit(ir::vaEnd());
                    } else if constexpr (std::is_same_v<T, symbols::VaCopyPlan>) {
                        emit(ir::vaCopy(id(*args[0]->getResultSymbol(store_)),
                                id(*args[1]->getResultSymbol(store_))));
                    } else {
                        emit(ir::vaArg(id(*args[0]->getResultSymbol(store_)),
                                id(*functionCall.getResultSymbol(store_))));
                    }
                } else {
                    functionCall.visitOperand(*this);
                    functionCall.visitArguments(*this);
                    for (auto& expression : functionCall.getArgumentList()) {
                        emit(ir::argument(convertedResult(*expression)));
                    }
                    int memoryReturnDest = kNoSymbol;
                    if (functionCall.hasResultSymbol(store_) && !functionCall.getType().isVoid()) {
                        if (type::object_abi::typeNeedsMemoryReturn(functionCall.getType())) {
                            memoryReturnDest = id(*functionCall.getResultSymbol(store_));
                        }
                    }
                    emit(ir::call(id(symbols::callCalleeName(*plan)), symbols::isIndirectCall(*plan),
                            memoryReturnDest));
                    if (functionCall.hasResultSymbol(store_) && !functionCall.getType().isVoid()) {
                        emit(ir::retrieve(id(*functionCall.getResultSymbol(store_)),
                                memoryReturnDest >= 0));
                    }
                }
            },
            *plan);
}

void CodeGeneratingVisitor::visit(ast::IdentifierExpression& identifier) {
    if (identifier.hasStringConstantLabel()) {
        assert(identifier.hasResultSymbol(store_) && "__func__ needs Result temp");
        emit(ir::assignLabelAddress(
                id(identifier.getStringConstantLabel()), id(*identifier.getResultSymbol(store_))));
        return;
    }
    type::IntegerConstant ice;
    if (identifier.evaluateConstant(ice)) {
        assert(identifier.hasResultSymbol(store_) && "folded enumerator needs Result temp");
        emitIntegerConstant(ice, id(*identifier.getResultSymbol(store_)));
        return;
    }
    // Function designators: plan holds the label; Result is the address temp.
    if (const auto* plan = store_.addressPlan(&identifier)) {
        if (const auto* d = symbols::get_if<symbols::FunctionDesignatorPlan>(plan)) {
            if (d->functionName) {
                assert(identifier.hasResultSymbol(store_) && "designator Result required for FunctionAddress");
                emit(ir::functionAddress(
                        id(*d->functionName), id(*identifier.getResultSymbol(store_))));
                return;
            }
        }
    }
    assert(!identifier.holdsFunctionDesignator()
            && "designator form without FunctionDesignatorPlan on the store");
}

void CodeGeneratingVisitor::visit(ast::ConstantExpression& constant) {
    // Decode to a numeric immediate so suffixes never reach the assembler raw.
    const int resultName = id(*constant.getResultSymbol(store_));
    if (type::isFloating(constant.expressionType())) {
        util::FloatingBits parsed;
        if (!util::floatingLiteralBits(constant.getValue(), parsed)) {
            throw std::runtime_error { "invalid floating constant: " + constant.getValue() };
        }
        emitFloatingConstant(resultName, parsed);
        return;
    }
    type::IntegerConstant value;
    if (!constant.evaluateConstant(value)) {
        throw std::runtime_error { "invalid integer constant: " + constant.getValue() };
    }
    emitIntegerConstant(value, resultName);
}

void CodeGeneratingVisitor::visit(ast::StringLiteralExpression& stringLiteral) {
    emit(ir::assignLabelAddress(
            id(stringLiteral.getConstantSymbol()), id(*stringLiteral.getResultSymbol(store_))));
}

void CodeGeneratingVisitor::emitIntegerConstant(const type::IntegerConstant& value,
        int dest) {
    const int lo = id(util::wordImmediate(type::bitsWord(value, 0)));
    if (type::object_abi::valueWords(value.type.getSize()) > 1) {
        emit(ir::assignConstant(lo, id(util::wordImmediate(type::bitsWord(value, 1))), dest));
        return;
    }
    emit(ir::assignConstant(lo, dest));
}

void CodeGeneratingVisitor::emitFloatingConstant(int dest, const util::FloatingBits& bits) {
    const int lo = id(util::hexImmediate(bits.bits));
    if (bits.sizeBytes > 8) {
        emit(ir::assignConstant(lo, id(util::hexImmediate(bits.bitsHi)), dest));
    } else {
        emit(ir::assignConstant(lo, dest));
    }
}

void CodeGeneratingVisitor::emitIncDec(int name, const type::Type& valueType, bool increment) {
    if (type::isFloating(valueType)) {
        const int one = addScratchValue(valueType);
        emitFloatingConstant(one, util::floatingOne(valueType.getSize()));
        if (increment) {
            emit(ir::add(name, one, name));
        } else {
            emit(ir::sub(name, one, name));
        }
        return;
    }
    if (valueType.isPointer()) {
        const int one = addScratchValue(type::signedInteger());
        emit(ir::assignConstant(id("1"), one));
        emitAdditive(increment ? '+' : '-', valueType, type::signedInteger(), name, one, name);
        return;
    }
    if (increment) {
        emit(ir::inc(name, 1));
    } else {
        emit(ir::dec(name, 1));
    }
}

void CodeGeneratingVisitor::visit(ast::PostfixExpression& expression) {
    expression.visitOperand(*this);

    auto* pre = expression.getPreOperationSymbol(store_);
    assert(pre && "Postfix PreOperation required after successful SA");
    const int resultSymbolName = id(*expression.getResultSymbol(store_));
    const int preOperationSymbol = id(*pre);
    emit(ir::assign(resultSymbolName, preOperationSymbol));

    emitIncDec(resultSymbolName, expression.getResultSymbol(store_)->getType(),
            expression.lexeme() == "++");

    // Dereference (and similar) lvalues: value lives in a temp; store new value through the pointer.
    if (expression.operandLvalueSymbol(store_)) {
        emitLvalueStore(*expression.getOperandExpression(), resultSymbolName);
    }

    expression.setResultSymbol(store_, *pre);
}

void CodeGeneratingVisitor::visit(ast::PrefixExpression& expression) {
    expression.visitOperand(*this);

    const int resultSymbolName = id(*expression.getResultSymbol(store_));
    emitIncDec(resultSymbolName, expression.getResultSymbol(store_)->getType(),
            expression.lexeme() == "++");

    if (expression.operandLvalueSymbol(store_)) {
        emitLvalueStore(*expression.getOperandExpression(), resultSymbolName);
    }
}

void CodeGeneratingVisitor::visit(ast::UnaryExpression& expression) {
    if (expression.lexeme() == "sizeof") {
        if (expression.getSizeofValue() >= 0) {
            emit(ir::assignConstant(
                    id(std::to_string(expression.getSizeofValue())),
                    id(*expression.getResultSymbol(store_))));
            return;
        }
        emitSizeofProduct(expression.operandType(),
                id(*expression.getResultSymbol(store_)));
        return;
    }

    expression.visitOperand(*this);

    switch (expression.lexeme().front()) {
    case '&':
        // &function designator: SA reuses the designator temp (already emitted FunctionAddress).
        if (expression.getOperandExpression()->holdsFunctionDesignator()) {
            break;
        } else if (auto* lvalue = expression.operandLvalueSymbol(store_)) {
            // &a[i] / &*p: address is already computed in the operand's lvalue temp.
            emit(ir::assign(id(*lvalue), id(*expression.getResultSymbol(store_))));
        } else {
            emitArrayObjectAddress(*expression.operandSymbol(store_),
                    id(*expression.getResultSymbol(store_)));
        }
        break;
    case '*':
        if (expression.operandSymbol(store_)->getType().isPointer()) {
            // *fp for pointer-to-function: SA keeps the pointer value (no memory load).
            if (type::isPointerToBareFunction(expression.operandSymbol(store_)->getType())) {
                if (expression.operandSymbol(store_)->getName() != expression.getResultSymbol(store_)->getName()) {
                    emit(ir::assign(
                            id(*expression.operandSymbol(store_)), id(*expression.getResultSymbol(store_))));
                }
                break;
            }
            // Already an address (pointer or multi-dim decayed row).
            if (expression.getResultSymbol(store_)->getName() == expression.getLvalueSymbol(store_)->getName()) {
                // Address-only multi-dim *a: just materialize &array into the temp if needed.
                // Result and lvalue share the address temp; operand is the array object.
                if (expression.operandType().isArray()) {
                    emitArrayObjectAddress(*expression.operandSymbol(store_),
                            id(*expression.getLvalueSymbol(store_)));
                } else {
                    emit(ir::assign(
                            id(*expression.operandSymbol(store_)), id(*expression.getResultSymbol(store_))));
                }
            } else {
                emit(ir::dereference(id(*expression.operandSymbol(store_)),
                        id(*expression.getLvalueSymbol(store_)), id(*expression.getResultSymbol(store_))));
            }
        } else if (expression.operandType().isArray()) {
            emitArrayObjectAddress(*expression.operandSymbol(store_),
                    id(*expression.getLvalueSymbol(store_)));
            if (expression.getResultSymbol(store_)->getName() != expression.getLvalueSymbol(store_)->getName()) {
                const int addr = id(*expression.getLvalueSymbol(store_));
                emit(ir::dereference(addr, addr, id(*expression.getResultSymbol(store_))));
            }
        } else {
            emit(ir::dereference(id(*expression.operandSymbol(store_)),
                    id(*expression.getLvalueSymbol(store_)), id(*expression.getResultSymbol(store_))));
        }
        break;
    case '+':
        emit(ir::assign(
                convertedResult(*expression.getOperandExpression()),
                id(*expression.getResultSymbol(store_))));
        break;
    case '-':
        emit(ir::unaryMinus(convertedResult(*expression.getOperandExpression()),
                id(*expression.getResultSymbol(store_))));
        break;
    case '~':
        emit(ir::unaryNot(convertedResult(*expression.getOperandExpression()),
                id(*expression.getResultSymbol(store_))));
        break;
    case '!':
        emit(ir::zeroCompare(id(*expression.operandSymbol(store_))));
        emit(ir::jump(id(*expression.getTruthyLabel(store_)), JumpCondition::IF_EQUAL));
        emit(ir::assignConstant(id("0"), id(*expression.getResultSymbol(store_))));
        emit(ir::jump(id(*expression.getFalsyLabel(store_))));
        emit(ir::label(id(*expression.getTruthyLabel(store_))));
        emit(ir::assignConstant(id("1"), id(*expression.getResultSymbol(store_))));
        emit(ir::label(id(*expression.getFalsyLabel(store_))));
        break;
    default:
        throw std::runtime_error { "Unidentified unary operator: " + expression.lexeme() };
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
        emitStructFieldInits(id(*object), fieldStores);
        return;
    }
    if (expression.initializer().hasResultSymbol(store_)) {
        emit(ir::assign(convertedResult(expression.initializer()), id(*object)));
    }
}

void CodeGeneratingVisitor::visit(ast::TypeNameExpression&) {
}

void CodeGeneratingVisitor::visit(ast::TypeCast& expression) {
    expression.visitOperand(*this);
    auto* source = expression.operandSymbol(store_);
    auto* dest = expression.getResultSymbol(store_);
    // Only true array objects need AddressOf. Multi-dim rows already hold a decayed pointer
    // in the result symbol while expression type may still be array.
    if (source->getType().isArray()) {
        emit(ir::addressOf(id(*source), id(*dest)));
    } else {
        emitConvert(id(*source), id(*dest), source->getType(), dest->getType());
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
    const char op = expression.lexeme().front();
    const int leftName = convertedResult(*expression.getLeftOperand());
    const int rightName = convertedResult(*expression.getRightOperand());
    const int resultName = id(*resultSym);

    if (op == '+' || op == '-') {
        emitAdditive(op, leftType, rightType, leftName, rightName, resultName);
        return;
    }
    if (op == '*' || op == '/' || op == '%') {
        emitMulDiv(op, leftName, rightName, resultName, resultSym->getType());
        return;
    }
    throw std::runtime_error { "unidentified arithmetic operator: " + expression.lexeme() };
}

void CodeGeneratingVisitor::emitAdditive(char op, const type::Type& leftType, const type::Type& rightType,
        int leftName, int rightName, int resultName) {
    const type::PointerArithmeticInfo ptrArith = type::classifyPointerArithmetic(leftType, rightType, op);
    switch (ptrArith.form) {
    case type::PointerArithmeticForm::None:
        break;
    case type::PointerArithmeticForm::PtrPlusInt:
    case type::PointerArithmeticForm::IntPlusPtr:
    case type::PointerArithmeticForm::PtrMinusInt: {
        const bool intLeft = ptrArith.form == type::PointerArithmeticForm::IntPlusPtr;
        const bool subtract = ptrArith.form == type::PointerArithmeticForm::PtrMinusInt;
        const int pointerName = intLeft ? rightName : leftName;
        const int indexName = intLeft ? leftName : rightName;
        const type::Type pointee = (intLeft ? rightType : leftType).dereference();
        const ScaledIndex scaled = scaleIndex(pointee, indexName, ptrArith.strideBytes);
        emit(ir::pointerOffset(pointerName, scaled.name, scaled.strideBytes, resultName, subtract));
        return;
    }
    case type::PointerArithmeticForm::PtrMinusPtr: {
        const type::Type pointee = leftType.dereference();
        if (type::hasComputableRuntimeSize(pointee)) {
            const int size = addScratchValue(type::signedInteger());
            emitSizeofProduct(pointee, size);
            const int bytes = addScratchValue(type::signedInteger());
            emit(ir::pointerDiff(leftName, rightName, 1, bytes));
            emitIntegerMulDiv('/', bytes, size, resultName, type::signedInteger());
            return;
        }
        emit(ir::pointerDiff(leftName, rightName, ptrArith.strideBytes, resultName));
        return;
    }
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

    const int leftName = convertedResult(*expression.getLeftOperand());
    const int rightName = convertedResult(*expression.getRightOperand());
    const int resultName = id(*expression.getResultSymbol(store_));
    switch (expression.lexeme().front()) {
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
            convertedResult(*expression.getLeftOperand()),
            convertedResult(*expression.getRightOperand()),
            signedRel));

    const int truthyLabel = id(*expression.getTruthyLabel(store_));
    if (expression.lexeme() == ">") {
        emit(ir::jump(truthyLabel, JumpCondition::IF_ABOVE, signedRel));
    } else if (expression.lexeme() == "<") {
        emit(ir::jump(truthyLabel, JumpCondition::IF_BELOW, signedRel));
    } else if (expression.lexeme() == "<=") {
        emit(ir::jump(truthyLabel, JumpCondition::IF_BELOW_OR_EQUAL, signedRel));
    } else if (expression.lexeme() == ">=") {
        emit(ir::jump(truthyLabel, JumpCondition::IF_ABOVE_OR_EQUAL, signedRel));
    } else if (expression.lexeme() == "==") {
        emit(ir::jump(truthyLabel, JumpCondition::IF_EQUAL));
    } else if (expression.lexeme() == "!=") {
        emit(ir::jump(truthyLabel, JumpCondition::IF_NOT_EQUAL));
    } else {
        throw std::runtime_error { "unidentified ml_op operator!\n" };
    }

    emit(ir::assignConstant(id("0"), id(*expression.getResultSymbol(store_))));
    emit(ir::jump(id(*expression.getFalsyLabel(store_))));
    emit(ir::label(truthyLabel));
    emit(ir::assignConstant(id("1"), id(*expression.getResultSymbol(store_))));
    emit(ir::label(id(*expression.getFalsyLabel(store_))));
}

void CodeGeneratingVisitor::visit(ast::BitwiseExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    const int leftName = convertedResult(*expression.getLeftOperand());
    const int rightName = convertedResult(*expression.getRightOperand());
    const int resultName = id(*expression.getResultSymbol(store_));
    switch (expression.lexeme().front()) {
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
        throw std::runtime_error { "no semantic actions defined for bitwise operator: " + expression.lexeme() };
    }
}

void CodeGeneratingVisitor::visit(ast::LogicalAndExpression& expression) {
    expression.visitLeftOperand(*this);

    emit(ir::assignConstant(id("0"), id(*expression.getResultSymbol(store_))));
    emit(ir::zeroCompare(id(*expression.leftOperandSymbol(store_))));
    emit(ir::jump(id(*expression.getExitLabel(store_)), JumpCondition::IF_EQUAL));

    expression.visitRightOperand(*this);

    emit(ir::zeroCompare(id(*expression.rightOperandSymbol(store_))));
    emit(ir::jump(id(*expression.getExitLabel(store_)), JumpCondition::IF_EQUAL));
    emit(ir::assignConstant(id("1"), id(*expression.getResultSymbol(store_))));

    emit(ir::label(id(*expression.getExitLabel(store_))));
}

void CodeGeneratingVisitor::visit(ast::LogicalOrExpression& expression) {
    expression.visitLeftOperand(*this);

    emit(ir::assignConstant(id("1"), id(*expression.getResultSymbol(store_))));
    emit(ir::zeroCompare(id(*expression.leftOperandSymbol(store_))));
    emit(ir::jump(id(*expression.getExitLabel(store_)), JumpCondition::IF_NOT_EQUAL));

    expression.visitRightOperand(*this);

    emit(ir::zeroCompare(id(*expression.rightOperandSymbol(store_))));
    emit(ir::jump(id(*expression.getExitLabel(store_)), JumpCondition::IF_NOT_EQUAL));
    emit(ir::assignConstant(id("0"), id(*expression.getResultSymbol(store_))));

    emit(ir::label(id(*expression.getExitLabel(store_))));
}

void CodeGeneratingVisitor::visit(ast::ConditionalExpression& expression) {
    expression.visitCondition(*this);
    emit(ir::zeroCompare(id(*expression.conditionSymbol(store_))));
    emit(ir::jump(id(*expression.getFalsyLabel(store_)), JumpCondition::IF_EQUAL));

    expression.visitTrueExpression(*this);
    emit(ir::assign(
            convertedResult(*expression.getTrueExpression()),
            id(*expression.getResultSymbol(store_))));
    emit(ir::jump(id(*expression.getExitLabel(store_))));

    emit(ir::label(id(*expression.getFalsyLabel(store_))));
    expression.visitFalseExpression(*this);
    emit(ir::assign(
            convertedResult(*expression.getFalseExpression()),
            id(*expression.getResultSymbol(store_))));

    emit(ir::label(id(*expression.getExitLabel(store_))));
}

void CodeGeneratingVisitor::visit(ast::AssignmentExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    const auto& op = expression.lexeme();
    const int resultName = id(*expression.getResultSymbol(store_));
    const int rightName = convertedResult(*expression.getRightOperand());
    if (op == "+=" || op == "-=") {
        const type::Type leftType = expression.getResultSymbol(store_)->getType();
        const type::Type rightType = expression.rightOperandSymbol(store_)->getType();
        emitAdditive(op.front(), leftType, rightType,
                resultName, rightName, resultName);
    }
    else if (op == "*="
            || op == "/="
            || op == "%=") {
        emitMulDiv(op.front(), resultName, rightName, resultName,
                expression.getResultSymbol(store_)->getType());
    }
    else if (op == "&=")
        emit(ir::andOp(resultName, rightName, resultName));
    else if (op == "^=")
        emit(ir::xorOp(resultName, rightName, resultName));
    else if (op == "|=")
        emit(ir::orOp(resultName, rightName, resultName));
    else if (op == "<<=") {
        emit(ir::shl(resultName, rightName, resultName));
    } else if (op == ">>=") {
        emit(ir::shr(resultName, rightName, resultName,
                type::valueIsSigned(expression.getResultSymbol(store_)->getType())));
    } else if (op == "=") {
        if (expression.leftOperandLvalueSymbol(store_)) {
            emit(ir::assign(rightName, resultName));
            emitLvalueStore(*expression.getLeftOperand(), resultName);
        } else {
            emit(ir::assign(rightName, resultName));
        }
        return;
    } else {
        throw std::runtime_error { "unidentified assignment operator: " + op };
    }

    if (expression.leftOperandLvalueSymbol(store_)) {
        emitLvalueStore(*expression.getLeftOperand(), resultName);
    }
}

void CodeGeneratingVisitor::visit(ast::ExpressionList& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);
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
        values.push_back(valueFromSymbol(module_.strings, valueSymbol.second));
    }
    std::vector<Value> arguments;
    for (auto& argumentSymbol : function.getArguments()) {
        arguments.push_back(valueFromSymbol(module_.strings, argumentSymbol));
    }
    Procedure procedure;
    procedure.name = id(function.getSymbol()->getName());
    procedure.frame.locals = std::move(values);
    procedure.frame.arguments = std::move(arguments);
    const bool variadic = function.getSymbol()->getType().isVariadic();
    procedure.memoryReturn = type::object_abi::typeNeedsMemoryReturn(
            function.getSymbol()->returnType());
    procedure.variadic = variadic;
    procedure.exported = !function.getSymbol()->hasInternalLinkage();
    internProcedureTemps(module_.strings, procedure);

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

