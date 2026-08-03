#include "CodeGeneratingVisitorInternal.h"

#include <stdexcept>
#include <variant>

namespace codegen {
namespace {

void requireTemps(const symbols::FieldInitTemps& temps, const char* what) {
    if (!temps.source || !temps.address) {
        throw std::logic_error { std::string(what) + " missing SA temps" };
    }
}

void emitTypedStore(CodeGeneratingVisitor& cg, const std::string& baseName, int offsetBytes,
        const type::Type& storeType, const std::string& valueName, const symbols::FieldInitTemps& temps,
        const std::optional<type::BitField>& bits = {}) {
    requireTemps(temps, "FieldInit store");
    if (storeType.isVoid()) {
        throw std::logic_error { "FieldInit store row missing storeType" };
    }
    cg.emit(ir::fieldAddress(baseName, offsetBytes, temps.address->getName(),
            symbols::AddressBaseMode::LeaObject));
    if (bits) {
        cg.emitBitFieldInsert(temps.address->getName(), valueName, *bits, storeType);
        return;
    }
    cg.emit(ir::lvalueAssign(valueName, temps.address->getName(),
            type::memoryAccessSizeBytes(storeType)));
}

} // namespace

namespace code_gen_detail {

void emitFieldInits(CodeGeneratingVisitor& cg,
        const std::string& baseName,
        const std::vector<symbols::FieldInit>& fields) {
    std::string sharedZeroName;
    for (const auto& field : fields) {
        if (const auto* zero = std::get_if<symbols::FieldZeroSpan>(&field)) {
            requireTemps(zero->temps, "FieldInit");
            if (sharedZeroName.empty()) {
                sharedZeroName = zero->temps.source->getName();
                cg.emit(ir::assignConstant("0", sharedZeroName));
            }
            int remaining = zero->zeroSpanBytes;
            int off = 0;
            while (remaining > 0) {
                const int chunk = remaining >= 8 ? 8 : (remaining >= 4 ? 4 : (remaining >= 2 ? 2 : 1));
                cg.emit(ir::fieldAddress(baseName, zero->offsetBytes + off, zero->temps.address->getName(),
                        symbols::AddressBaseMode::LeaObject));
                cg.emit(ir::lvalueAssign(sharedZeroName, zero->temps.address->getName(), chunk));
                off += chunk;
                remaining -= chunk;
            }
            continue;
        }
        if (const auto* c = std::get_if<symbols::FieldConstant>(&field)) {
            requireTemps(c->temps, "FieldInit");
            cg.emit(ir::assignConstant(c->constantValue, c->temps.source->getName()));
            emitTypedStore(cg, baseName, c->offsetBytes, c->storeType,
                    c->temps.source->getName(), c->temps, c->bitField);
            continue;
        }
        if (const auto* a = std::get_if<symbols::FieldAddressOf>(&field)) {
            requireTemps(a->temps, "FieldInit");
            cg.emit(ir::addressOf(a->addressOfOperand, a->temps.source->getName()));
            emitTypedStore(cg, baseName, a->offsetBytes, a->storeType,
                    a->temps.source->getName(), a->temps);
            continue;
        }
        if (const auto* v = std::get_if<symbols::FieldValue>(&field)) {
            requireTemps(v->temps, "FieldInit");
            emitTypedStore(cg, baseName, v->offsetBytes, v->storeType,
                    v->temps.source->getName(), v->temps, v->bitField);
            continue;
        }
        if (const auto* s = std::get_if<symbols::FieldStringBytes>(&field)) {
            requireTemps(s->temps, "FieldInit");
            const std::string valueName = s->temps.source->getName();
            const std::string addrName = s->temps.address->getName();
            int off = 0;
            while (off < s->sizeBytes) {
                const int remaining = s->sizeBytes - off;
                const int chunk = remaining >= 8 ? 8 : (remaining >= 4 ? 4 : 1);
                unsigned long long word = 0;
                for (int i = 0; i < chunk; ++i) {
                    const int idx = off + i;
                    const unsigned char b = (idx < static_cast<int>(s->bytes.size()))
                            ? s->bytes[static_cast<std::size_t>(idx)] : 0;
                    word |= static_cast<unsigned long long>(b) << (8 * i);
                }
                cg.emit(ir::assignConstant(std::to_string(static_cast<long long>(word)), valueName));
                cg.emit(ir::fieldAddress(baseName, s->offsetBytes + off, addrName,
                        symbols::AddressBaseMode::LeaObject));
                cg.emit(ir::lvalueAssign(valueName, addrName, chunk));
                off += chunk;
            }
        }
    }
}

} // namespace code_gen_detail
} // namespace codegen
