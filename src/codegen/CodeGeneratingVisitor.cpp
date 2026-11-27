#include "CodeGeneratingVisitor.h"
#include "codegen/InternalError.h"
#include "ast/Expression.h"
#include "codegen/IrBuilders.h"

#include <utility>

#include "symbols/ValueEntry.h"
#include "types/TypeQuery.h"

namespace codegen {

namespace {

const char* floatingSuffix(const type::Type& t) {
    if (type::isLongDouble(t)) {
        return "xf";
    }
    return type::isDouble(t) ? "df" : "sf";
}

bool isWideInteger(const type::Type& t) {
    return type::isIntegral(t) && type::object_abi::valueWords(t.getSize()) > 1;
}

std::string wideIntegerFloatingHelper(const type::Type& from, const type::Type& to) {
    if (isWideInteger(from) && type::isFloating(to)) {
        return std::string("__float") + (type::valueIsSigned(from) ? "" : "un") + "ti" + floatingSuffix(to);
    }
    if (type::isFloating(from) && isWideInteger(to)) {
        return std::string("__fix") + (type::valueIsSigned(to) ? "" : "uns") + floatingSuffix(from) + "ti";
    }
    return {};
}

} // namespace

CodeGeneratingVisitor::CodeGeneratingVisitor(symbols::AnnotationStore& store,
        const ast::VlaExpressionTable* vlas) : store_ { store }, vlas_ { vlas } {
}

void CodeGeneratingVisitor::emit(Instruction instruction) {
    if (!currentBody_) {
        internalError("CodeGeneratingVisitor: emit outside of a procedure body");
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

void CodeGeneratingVisitor::emitAssignUnlessSame(int src, int dest) {
    if (src != dest) {
        emit(ir::assign(src, dest));
    }
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
    if ((type::isIntegral(sourceType) && type::isComplex(destType))
            || (type::isComplex(sourceType) && type::isIntegral(destType))) {
        const type::Type real = type::correspondingReal(type::isComplex(destType) ? destType : sourceType);
        const int part = addScratchValue(real);
        emitConvert(source, part, sourceType, real);
        emitConvert(part, dest, real, destType);
        return;
    }
    if (const std::string helper = wideIntegerFloatingHelper(sourceType, destType); !helper.empty()) {
        emit(ir::argument(source));
        emit(ir::call(id(helper)));
        emit(ir::retrieve(dest));
        return;
    }
    if (type::needsIntegerWiden(sourceType, destType)
            || type::needsIntegerToPointerExtend(sourceType, destType)) {
        emit(ir::widen(source, dest, type::valueIsSigned(sourceType)));
        return;
    }
    const auto unsigned64 = [](const type::Type& t) {
        return type::isIntegral(t) && !type::valueIsSigned(t) && t.getSize() == 8;
    };
    if ((unsigned64(sourceType) && type::isFloating(destType))
            || (type::isFloating(sourceType) && unsigned64(destType))) {
        Instruction cast = ir::assign(source, dest);
        cast.imm = 1;
        emit(cast);
        return;
    }
    emitAssignUnlessSame(source, dest);
}

void CodeGeneratingVisitor::emitIntegerMulDiv(type::ArithmeticOp op, int left,
        int right, int result, const type::Type& resultType) {
    if (isWideInteger(resultType)) {
        const char* helper = "__multi3";
        switch (op) {
        case type::ArithmeticOp::Mul:
            break;
        case type::ArithmeticOp::Div:
            helper = type::valueIsSigned(resultType) ? "__divti3" : "__udivti3";
            break;
        case type::ArithmeticOp::Mod:
            helper = type::valueIsSigned(resultType) ? "__modti3" : "__umodti3";
            break;
        case type::ArithmeticOp::Add:
        case type::ArithmeticOp::Sub:
            internalError("emitIntegerMulDiv: additive op");
        }
        emit(ir::argument(left));
        emit(ir::argument(right));
        emit(ir::call(id(helper)));
        emit(ir::retrieve(result));
        return;
    }
    switch (op) {
    case type::ArithmeticOp::Mul:
        emit(ir::mul(left, right, result));
        break;
    case type::ArithmeticOp::Div:
        emit(ir::div(left, right, result, type::valueIsSigned(resultType)));
        break;
    case type::ArithmeticOp::Mod:
        emit(ir::mod(left, right, result, type::valueIsSigned(resultType)));
        break;
    case type::ArithmeticOp::Add:
    case type::ArithmeticOp::Sub:
        internalError("emitIntegerMulDiv: additive op");
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

IntermediateRepresentation CodeGeneratingVisitor::takeIr() {
    return std::move(module_);
}

} // namespace codegen
