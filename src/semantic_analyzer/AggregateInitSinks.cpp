#include "AggregateInitSinks.h"

#include "SemanticAnalysisVisitorInternal.h"
#include "StaticInitFold.h"

#include "types/ObjectAbi.h"

#include <variant>

namespace semantic_analyzer {

FieldPlanSink::FieldPlanSink(SemanticAnalysisVisitor& v, SymbolTable& st,
        symbols::AnnotationStore& ann, translation_unit::Context ctx,
        std::vector<symbols::StructFieldInit>& p)
        : visitor { v }, symbolTable { st }, annotations { ann }, context { std::move(ctx) },
          plan { p } {
}

bool FieldPlanSink::ok() const {
    return !failed;
}

void FieldPlanSink::error(const std::string& message) {
    failed = true;
    visitor.semanticError(message, context);
}

void FieldPlanSink::onUnwritten(const type::FoundMember& slot) {
    if (slot.isBitField() || !slot.type.isAggregate()) {
        placeScalar(slot, nullptr);
        return;
    }
    forEachUnwrittenRepresentation(slot.type, slot.offsetBytes,
            [&](int off, const type::Type& storeType) {
                symbols::StructFieldInit field;
                field.offsetBytes = off;
                auto addr = symbolTable.createTemporarySymbol(type::pointer(storeType));
                field.addressName = addr.getName();
                auto zero = symbolTable.createTemporarySymbol(storeType);
                field.zeroInitialize = true;
                field.sourceName = zero.getName();
                plan.push_back(std::move(field));
            });
}

void FieldPlanSink::placeScalar(const type::FoundMember& slot, ast::Expression* value) {
    symbols::StructFieldInit field;
    field.offsetBytes = slot.offsetBytes;
    field.bitField = slot.bitField;
    field.type = slot.type;
    const type::Type& storeType = slot.type;
    auto addr = symbolTable.createTemporarySymbol(type::pointer(storeType));
    field.addressName = addr.getName();
    if (value && value->hasResultSymbol(annotations)) {
        decayArrayToPointer(*value, storeType, symbolTable, annotations);
        const type::Type src = assignSourceType(*value, storeType, annotations);
        if (!visitor.checkAssign(storeType, src, context, value)) {
            failed = true;
        }
        maybeSetConversion(value, storeType, symbolTable, annotations);
        field.zeroInitialize = false;
        if (auto* converted = annotations.conversion(value)) {
            field.sourceName = converted->getName();
        } else {
            field.sourceName = value->getResultSymbol(annotations)->getName();
        }
    } else {
        auto zero = symbolTable.createTemporarySymbol(storeType);
        field.zeroInitialize = true;
        field.sourceName = zero.getName();
    }
    plan.push_back(std::move(field));
}

void FieldPlanSink::placeInteger(const type::FoundMember& slot, long value) {
    symbols::StructFieldInit field;
    field.offsetBytes = slot.offsetBytes;
    field.bitField = slot.bitField;
    field.type = slot.type;
    auto addr = symbolTable.createTemporarySymbol(type::pointer(slot.type));
    field.addressName = addr.getName();
    auto src = symbolTable.createTemporarySymbol(slot.type);
    field.sourceName = src.getName();
    field.immediate = std::to_string(value);
    plan.push_back(std::move(field));
}

DataWordSink::DataWordSink(SemanticAnalysisVisitor& v, translation_unit::Context ctx,
        std::vector<symbols::StaticInitValue>& w, int wc)
        : visitor { v }, context { std::move(ctx) }, words { w }, wordCount { wc } {
}

bool DataWordSink::ok() const {
    return !failed;
}

void DataWordSink::error(const std::string& message) {
    failed = true;
    visitor.semanticError(message, context);
}

namespace {

unsigned long long numericBits(const symbols::StaticInitValue& word) {
    if (auto* bits = std::get_if<symbols::StaticWord>(&word)) {
        return bits->bits;
    }
    if (auto* integer = std::get_if<symbols::StaticInteger>(&word)) {
        return static_cast<unsigned long long>(integer->value.bits);
    }
    if (auto* fp = std::get_if<symbols::StaticFloat>(&word)) {
        return fp->bits;
    }
    return 0;
}

void storeBitsAt(std::vector<symbols::StaticInitValue>& words, int wordCount, int offsetBytes,
        unsigned long long value, int storeSizeBytes) {
    if (offsetBytes < 0 || storeSizeBytes <= 0) {
        return;
    }
    const int wi = type::object_abi::wordIndexAt(offsetBytes);
    if (wi < 0 || wi >= wordCount) {
        return;
    }
    auto& word = words[static_cast<std::size_t>(wi)];
    if (storeSizeBytes >= type::object_abi::MACHINE_WORD_SIZE) {
        word = symbols::StaticWord { value };
        return;
    }
    unsigned long long wordVal = numericBits(word);
    const int lane = offsetBytes % type::object_abi::MACHINE_WORD_SIZE;
    const int bits = storeSizeBytes * 8;
    const unsigned long long mask = bits >= 64 ? ~0ull : ((1ull << bits) - 1ull);
    wordVal &= ~(mask << (lane * 8));
    wordVal |= (value & mask) << (lane * 8);
    word = symbols::StaticWord { wordVal };
}

void storeAddressAt(std::vector<symbols::StaticInitValue>& words, int wordCount, int offsetBytes,
        symbols::StaticInitValue value) {
    if (offsetBytes < 0) {
        return;
    }
    const int wi = type::object_abi::wordIndexAt(offsetBytes);
    if (wi < 0 || wi >= wordCount) {
        return;
    }
    words[static_cast<std::size_t>(wi)] = std::move(value);
}

} // namespace

void DataWordSink::onUnwritten(const type::FoundMember& slot) {
    if (slot.isBitField()) {
        return;
    }
    if (!slot.type.isAggregate()) {
        storeBitsAt(words, wordCount, slot.offsetBytes, 0, slot.type.getSize());
        return;
    }
    forEachUnwrittenRepresentation(slot.type, slot.offsetBytes,
            [&](int off, const type::Type& storeType) {
                storeBitsAt(words, wordCount, off, 0, storeType.getSize());
            });
}

void DataWordSink::placeScalar(const type::FoundMember& slot, ast::Expression* value) {
    const int offsetBytes = slot.offsetBytes;
    const type::Type& storeType = slot.type;
    if (!value) {
        return;
    }
    auto folded = evaluateStaticInit(visitor, *value, storeType, context);
    if (!folded) {
        failed = true;
        return;
    }
    if (std::holds_alternative<symbols::StaticAddress>(*folded)) {
        storeAddressAt(words, wordCount, offsetBytes, std::move(*folded));
        return;
    }
    unsigned long long bits = 0;
    if (auto* integer = std::get_if<symbols::StaticInteger>(&*folded)) {
        bits = static_cast<unsigned long long>(integer->value.bits);
    } else if (auto* fp = std::get_if<symbols::StaticFloat>(&*folded)) {
        bits = fp->bits;
    }
    if (slot.isBitField()) {
        const auto& bf = *slot.bitField;
        const int absBit = slot.offsetBytes * 8 + bf.shift;
        const int wi = type::object_abi::wordIndexAt(absBit / 8);
        if (wi < 0 || wi >= wordCount) {
            return;
        }
        auto& word = words[static_cast<std::size_t>(wi)];
        unsigned long long wordVal = numericBits(word);
        const int shift = absBit % (type::object_abi::MACHINE_WORD_SIZE * 8);
        const unsigned long long mask = type::bitFieldMask(bf.width);
        wordVal &= ~(mask << shift);
        wordVal |= (bits & mask) << shift;
        word = symbols::StaticWord { wordVal };
        return;
    }
    const auto pieces = symbols::asDataWords(*folded);
    if (pieces.size() == 1) {
        storeBitsAt(words, wordCount, offsetBytes, numericBits(pieces.front()), storeType.getSize());
        return;
    }
    int off = offsetBytes;
    for (const auto& piece : pieces) {
        storeBitsAt(words, wordCount, off, numericBits(piece), type::object_abi::MACHINE_WORD_SIZE);
        off += type::object_abi::MACHINE_WORD_SIZE;
    }
}

void DataWordSink::placeInteger(const type::FoundMember& slot, long value) {
    storeBitsAt(words, wordCount, slot.offsetBytes, static_cast<unsigned long long>(value),
            slot.type.getSize());
}

} // namespace semantic_analyzer
