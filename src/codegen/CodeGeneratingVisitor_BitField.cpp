#include "CodeGeneratingVisitor.h"

#include "Instruction.h"
#include "util/ImmediateFormat.h"
#include "types/Type.h"

namespace codegen {

void CodeGeneratingVisitor::emitBitFieldExtract(int container, int dest,
        const type::BitField& bits) {
    const type::Type word = type::signedLong();
    const int shamt = addScratchValue(word);
    const int mask = addScratchValue(word);
    const int tmp = addScratchValue(word);
    emit(ir::assignConstant(id(std::to_string(bits.shift)), shamt));
    emit(ir::shr(container, shamt, tmp, false));
    emit(ir::assignConstant(id(util::hexImmediate(type::bitFieldMask(bits.width))), mask));
    emit(ir::andOp(tmp, mask, tmp));
    if (bits.isSigned && bits.width > 0 && bits.width < 64) {
        emit(ir::assignConstant(id(std::to_string(64 - bits.width)), shamt));
        emit(ir::shl(tmp, shamt, tmp));
        emit(ir::shr(tmp, shamt, dest, true));
    } else {
        emit(ir::assign(tmp, dest));
    }
}

void CodeGeneratingVisitor::emitBitFieldInsert(int addr, int value,
        const type::BitField& bits, const type::Type& unit) {
    const type::Type word = type::signedLong();
    const int cur = addScratchValue(unit);
    const int shamt = addScratchValue(word);
    const int tmp = addScratchValue(word);
    const int mask = addScratchValue(word);
    emit(ir::dereference(addr, addr, cur));
    const unsigned long long fieldMask = type::bitFieldMask(bits.width);
    emit(ir::assignConstant(id(util::hexImmediate(fieldMask)), mask));
    emit(ir::andOp(value, mask, tmp));
    emit(ir::assignConstant(id(std::to_string(bits.shift)), shamt));
    emit(ir::shl(tmp, shamt, tmp));
    const unsigned long long clear = ~(fieldMask << bits.shift);
    emit(ir::assignConstant(id(util::hexImmediate(clear)), mask));
    emit(ir::andOp(cur, mask, cur));
    emit(ir::orOp(cur, tmp, cur));
    emit(ir::lvalueAssign(cur, addr));
}

} // namespace codegen
