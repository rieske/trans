#include "CodeGeneratingVisitor.h"

#include "Instruction.h"
#include "util/ImmediateFormat.h"
#include "types/Type.h"

#include "ast/Expression.h"

namespace codegen {

namespace {

unsigned long long bitMask(int width) {
    if (width <= 0) {
        return 0;
    }
    if (width >= 64) {
        return ~0ull;
    }
    return (1ull << width) - 1ull;
}

} // namespace

void CodeGeneratingVisitor::emitBitFieldExtract(const std::string& container, const std::string& dest,
        const type::BitField& bits) {
    const type::Type word = type::signedLong();
    const std::string shamt = addScratchValue(word);
    const std::string mask = addScratchValue(word);
    const std::string tmp = addScratchValue(word);
    emit(ir::assignConstant(std::to_string(bits.shift), shamt));
    emit(ir::shr(container, shamt, tmp, false));
    emit(ir::assignConstant(util::hexImmediate(bitMask(bits.width)), mask));
    emit(ir::andOp(tmp, mask, tmp));
    if (bits.isSigned && bits.width > 0 && bits.width < 64) {
        emit(ir::assignConstant(std::to_string(64 - bits.width), shamt));
        emit(ir::shl(tmp, shamt, tmp));
        emit(ir::shr(tmp, shamt, dest, true));
    } else {
        emit(ir::assign(tmp, dest));
    }
}

void CodeGeneratingVisitor::emitBitFieldInsert(const std::string& addr, const std::string& value,
        const type::BitField& bits, const type::Type& unit) {
    const type::Type word = type::signedLong();
    const std::string cur = addScratchValue(unit);
    const std::string shamt = addScratchValue(word);
    const std::string tmp = addScratchValue(word);
    const std::string mask = addScratchValue(word);
    emit(ir::dereference(addr, addr, cur));
    const unsigned long long fieldMask = bitMask(bits.width);
    emit(ir::assignConstant(util::hexImmediate(fieldMask), mask));
    emit(ir::andOp(value, mask, tmp));
    emit(ir::assignConstant(std::to_string(bits.shift), shamt));
    emit(ir::shl(tmp, shamt, tmp));
    const unsigned long long clear = ~(fieldMask << bits.shift);
    emit(ir::assignConstant(util::hexImmediate(clear), mask));
    emit(ir::andOp(cur, mask, cur));
    emit(ir::orOp(cur, tmp, cur));
    emit(ir::lvalueAssign(cur, addr));
}

void CodeGeneratingVisitor::emitLvalueStore(ast::Expression& lhs, const std::string& valueName) {
    auto* lvalue = lhs.getLvalueSymbol(store_);
    if (!lvalue) {
        return;
    }
    const auto* plan = store_.addressPlan(&lhs);
    const auto* field = plan ? symbols::get_if<symbols::FieldPlan>(plan) : nullptr;
    if (field && field->isBitField()) {
        emitBitFieldInsert(lvalue->getName(), valueName, *field->bitField, lhs.getType());
        return;
    }
    emit(ir::lvalueAssign(valueName, lvalue->getName()));
}

} // namespace codegen
