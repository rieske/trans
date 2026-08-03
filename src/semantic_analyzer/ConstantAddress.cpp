#include "ConstantAddress.h"

#include "ast/ArithmeticExpression.h"
#include "ast/ArrayAccess.h"
#include "ast/CompoundLiteralExpression.h"
#include "ast/ConstantExpression.h"
#include "ast/IdentifierExpression.h"
#include "ast/InitializerListExpression.h"
#include "ast/MemberAccess.h"
#include "ast/Operator.h"
#include "ast/StringLiteralExpression.h"
#include "ast/TypeCast.h"
#include "ast/UnaryExpression.h"
#include "symbols/AddressPlan.h"
#include "types/IntegerConstant.h"
#include "types/ObjectAbi.h"
#include "types/TypeQuery.h"
#include "util/FloatingLiteral.h"

#include <optional>
#include <utility>

namespace semantic_analyzer {
namespace {

std::optional<symbols::AddressInit> foldAddress(
        const ast::Expression& expr, const symbols::AnnotationStore& store);
std::optional<symbols::AddressInit> foldDesignatorAddress(
        const ast::Expression& expr, const symbols::AnnotationStore& store);

const ast::Expression* peelTypeCasts(const ast::Expression* expr) {
    while (auto* cast = dynamic_cast<const ast::TypeCast*>(expr)) {
        expr = cast->getOperandExpression();
        if (!expr) {
            return nullptr;
        }
    }
    return expr;
}

const symbols::ValueEntry* staticObjectHome(
        const ast::IdentifierExpression& id, const symbols::AnnotationStore& store) {
    if (const auto* lv = store.value(&id, symbols::ValueSlot::Lvalue); lv && lv->isGlobal()) {
        return lv;
    }
    if (store.hasResult(&id)) {
        const symbols::ValueEntry* entry = store.result(&id);
        if (entry->isGlobal()) {
            return entry;
        }
    }
    return nullptr;
}

std::optional<symbols::AddressInit> functionLabel(
        const ast::Expression& expr, const symbols::AnnotationStore& store) {
    if (!expr.holdsFunctionDesignator()) {
        return std::nullopt;
    }
    const auto* designator = symbols::get_if<symbols::FunctionDesignatorPlan>(store.addressPlan(&expr));
    if (!designator || !designator->functionName) {
        return std::nullopt;
    }
    return symbols::AddressInit { *designator->functionName, 0 };
}

std::optional<symbols::AddressInit> foldIdentifierDesignator(
        const ast::IdentifierExpression& id, const symbols::AnnotationStore& store) {
    if (const std::string* label = store.string(&id, symbols::StringSlot::ConstantLabel)) {
        return symbols::AddressInit { *label, 0 };
    }
    if (auto fn = functionLabel(id, store)) {
        return fn;
    }
    if (const auto* decay = symbols::get_if<symbols::ArrayDecayPlan>(store.addressPlan(&id))) {
        if (!decay->objectName.empty()) {
            return symbols::AddressInit { decay->objectName, 0 };
        }
    }
    const symbols::ValueEntry* home = staticObjectHome(id, store);
    if (!home) {
        return std::nullopt;
    }
    return symbols::AddressInit { home->getName(), 0 };
}

std::optional<symbols::AddressInit> foldMemberDesignator(
        const ast::MemberAccess& member, const symbols::AnnotationStore& store) {
    const auto* field = symbols::get_if<symbols::FieldPlan>(store.addressPlan(&member));
    if (!field || field->isBitField()) {
        return std::nullopt;
    }
    auto base = member.isArrow() ? foldAddress(*member.getBase(), store)
                                 : foldDesignatorAddress(*member.getBase(), store);
    if (!base) {
        return std::nullopt;
    }
    base->offset += field->fieldOffsetBytes;
    return base;
}

std::optional<symbols::AddressInit> foldIndexDesignator(
        const ast::ArrayAccess& access, const symbols::AnnotationStore& store) {
    const auto* index = symbols::get_if<symbols::IndexPlan>(store.addressPlan(&access));
    if (!index) {
        return std::nullopt;
    }
    long i = 0;
    const ast::Expression& left = *access.getLeftOperand();
    const ast::Expression& right = *access.getRightOperand();
    const ast::Expression* indexExpr = &right;
    const ast::Expression* baseExpr = &left;
    if (index->baseOperand == symbols::BinaryOperand::Right) {
        indexExpr = &left;
        baseExpr = &right;
    }
    if (!indexExpr->foldToHostLong(i)) {
        return std::nullopt;
    }
    auto base = baseExpr->isArrayObjectType()
            ? foldDesignatorAddress(*baseExpr, store)
            : foldAddress(*baseExpr, store);
    if (!base) {
        return std::nullopt;
    }
    base->offset += i * static_cast<long>(index->elementSize);
    return base;
}

std::optional<symbols::AddressInit> foldPointerArithmetic(
        const ast::ArithmeticExpression& arith, const symbols::AnnotationStore& store) {
    const ast::OperatorKind op = arith.getOperator()->getKind();
    if (op != ast::OperatorKind::Add && op != ast::OperatorKind::Sub) {
        return std::nullopt;
    }
    const char opChar = (op == ast::OperatorKind::Add) ? '+' : '-';
    const ast::Expression& left = *arith.getLeftOperand();
    const ast::Expression& right = *arith.getRightOperand();
    const type::PointerArithmeticInfo info = type::classifyPointerArithmetic(
            type::afterLvalueConversion(left.valueType(store)),
            type::afterLvalueConversion(right.valueType(store)), opChar);
    if (info.form != type::PointerArithmeticForm::PtrPlusInt
            && info.form != type::PointerArithmeticForm::IntPlusPtr
            && info.form != type::PointerArithmeticForm::PtrMinusInt) {
        return std::nullopt;
    }

    const ast::Expression* addrExpr = &left;
    const ast::Expression* iceExpr = &right;
    long sign = 1;
    if (info.form == type::PointerArithmeticForm::IntPlusPtr) {
        addrExpr = &right;
        iceExpr = &left;
    } else if (info.form == type::PointerArithmeticForm::PtrMinusInt) {
        sign = -1;
    }

    auto addr = foldAddress(*addrExpr, store);
    long ice = 0;
    if (!addr || !iceExpr->foldToHostLong(ice)) {
        return std::nullopt;
    }
    addr->offset += sign * ice * static_cast<long>(info.strideBytes);
    return addr;
}

std::optional<symbols::AddressInit> foldCompoundLiteralHome(
        const ast::Expression& expr, const symbols::AnnotationStore& store) {
    auto* literal = dynamic_cast<const ast::CompoundLiteralExpression*>(&expr);
    if (!literal) {
        return std::nullopt;
    }
    const auto* home = store.value(literal, symbols::ValueSlot::Object);
    if (home && home->isGlobal()) {
        return symbols::AddressInit { home->getName(), 0 };
    }
    return std::nullopt;
}

std::optional<symbols::AddressInit> foldDesignatorAddress(
        const ast::Expression& expr, const symbols::AnnotationStore& store) {
    if (auto addr = foldCompoundLiteralHome(expr, store)) {
        return addr;
    }
    if (auto* id = dynamic_cast<const ast::IdentifierExpression*>(&expr)) {
        return foldIdentifierDesignator(*id, store);
    }
    if (auto* member = dynamic_cast<const ast::MemberAccess*>(&expr)) {
        return foldMemberDesignator(*member, store);
    }
    if (auto* access = dynamic_cast<const ast::ArrayAccess*>(&expr)) {
        return foldIndexDesignator(*access, store);
    }
    if (auto* unary = dynamic_cast<const ast::UnaryExpression*>(&expr)) {
        if (unary->getOperator()->getKind() == ast::OperatorKind::Deref
                && unary->getOperandExpression()) {
            return foldAddress(*unary->getOperandExpression(), store);
        }
    }
    return std::nullopt;
}

std::optional<symbols::AddressInit> foldAddress(
        const ast::Expression& expr, const symbols::AnnotationStore& store) {
    if (auto addr = foldCompoundLiteralHome(expr, store)) {
        return addr;
    }
    if (auto* literal = dynamic_cast<const ast::StringLiteralExpression*>(&expr)) {
        if (literal->getConstantSymbol().empty()) {
            return std::nullopt;
        }
        return symbols::AddressInit { literal->getConstantSymbol(), 0 };
    }
    if (auto* id = dynamic_cast<const ast::IdentifierExpression*>(&expr)) {
        auto addr = foldIdentifierDesignator(*id, store);
        if (!addr) {
            return std::nullopt;
        }
        if (store.string(id, symbols::StringSlot::ConstantLabel)
                || id->holdsFunctionDesignator()
                || id->isArrayObjectType()) {
            return addr;
        }
        return std::nullopt;
    }
    if (auto* unary = dynamic_cast<const ast::UnaryExpression*>(&expr)) {
        if (unary->getOperator()->getKind() == ast::OperatorKind::AddressOf
                && unary->getOperandExpression()) {
            return foldDesignatorAddress(*unary->getOperandExpression(), store);
        }
    } else if (auto* cast = dynamic_cast<const ast::TypeCast*>(&expr)) {
        if (!cast->getOperandExpression()) {
            return std::nullopt;
        }
        return foldAddress(*cast->getOperandExpression(), store);
    } else if (auto* arith = dynamic_cast<const ast::ArithmeticExpression*>(&expr)) {
        return foldPointerArithmetic(*arith, store);
    }
    if (expr.holdsAggregateAddress()) {
        return foldDesignatorAddress(expr, store);
    }
    return std::nullopt;
}

bool storeTypeCanHoldAddress(const type::Type& storeType) {
    return storeType.isPointer()
            || (type::isIntegral(storeType)
                    && storeType.getSize() >= type::object_abi::MACHINE_WORD_SIZE);
}

} // namespace

bool foldFloatingBits(const ast::Expression* expr, util::FloatingBits& out) {
    expr = peelTypeCasts(expr);
    if (!expr) {
        return false;
    }
    if (auto* constant = dynamic_cast<const ast::ConstantExpression*>(expr)) {
        if (constant->hasExpressionType() && type::isFloating(constant->expressionType())) {
            return util::floatingLiteralBits(constant->getValue(), out);
        }
        return false;
    }
    auto* unary = dynamic_cast<const ast::UnaryExpression*>(expr);
    if (!unary) {
        return false;
    }
    const ast::OperatorKind op = unary->getOperator()->getKind();
    if (op != ast::OperatorKind::Add && op != ast::OperatorKind::Sub) {
        return false;
    }
    if (!foldFloatingBits(unary->getOperandExpression(), out)) {
        return false;
    }
    if (op == ast::OperatorKind::Sub) {
        util::negateFloating(out);
    }
    return true;
}

symbols::GlobalInitializer floatingInit(const util::FloatingBits& bits) {
    if (bits.sizeBytes > type::object_abi::MACHINE_WORD_SIZE) {
        return symbols::MultiWordInit { {
                symbols::ConstantInit { static_cast<long>(bits.bits) },
                symbols::ConstantInit { static_cast<long>(bits.bitsHi) } } };
    }
    return symbols::ConstantInit { static_cast<long>(bits.bits) };
}

bool tryFoldGlobalInit(ast::Expression* expr, const type::Type& storeType,
        symbols::AnnotationStore& store, symbols::GlobalInitializer& out) {
    if (!expr) {
        return false;
    }
    if (auto* list = dynamic_cast<ast::InitializerListExpression*>(expr)) {
        if (list->getElements().size() == 1 && !list->getElements()[0].isDesignated()) {
            expr = list->getElements()[0].value.get();
            if (!expr) {
                return false;
            }
        }
    }
    util::FloatingBits fp;
    if (foldFloatingBits(expr, fp)) {
        if (type::isFloating(storeType)) {
            if (fp.sizeBytes != storeType.getSize()) {
                fp = util::encodeFloatingBits(util::decodeFloatingBits(fp), storeType.getSize());
            }
            out = floatingInit(fp);
            return true;
        }
        const double decoded = util::decodeFloatingBits(fp);
        if (type::isBoolean(storeType)) {
            out = symbols::ConstantInit { decoded != 0.0 };
            return true;
        }
        if (type::isIntegral(storeType)) {
            out = symbols::ConstantInit {
                    type::toHostLong(type::convert(
                            type::fromHostLong(static_cast<long>(decoded)), storeType)) };
            return true;
        }
        return false;
    }
    type::IntegerConstant ice;
    if (expr->evaluateConstant(ice)) {
        ice = type::convert(ice, storeType);
        if (type::isFloating(storeType)) {
            out = floatingInit(util::encodeFloatingBits(
                    static_cast<double>(type::toHostLong(ice)), storeType.getSize()));
            return true;
        }
        if (storeType.getSize() > type::object_abi::MACHINE_WORD_SIZE) {
            out = symbols::MultiWordInit { {
                    symbols::ConstantInit { static_cast<long>(type::bitsWord(ice, 0)) },
                    symbols::ConstantInit { static_cast<long>(type::bitsWord(ice, 1)) } } };
            return true;
        }
        out = symbols::ConstantInit { type::toHostLong(ice) };
        return true;
    }
    if (!storeTypeCanHoldAddress(storeType)) {
        return false;
    }
    if (auto addr = foldAddress(*expr, store)) {
        out = std::move(*addr);
        return true;
    }
    return false;
}

} // namespace semantic_analyzer
