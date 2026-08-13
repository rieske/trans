#include "AggregateInitSinks.h"

#include "ConstantAddress.h"
#include "InitializerLowering.h"
#include "SemanticAnalysisVisitorInternal.h"

#include "ast/StringLiteralExpression.h"
#include "types/ObjectAbi.h"
#include "types/TypeQuery.h"
#include "util/StringLiteralDecode.h"

#include <functional>
#include <optional>
#include <variant>

namespace semantic_analyzer {

namespace {

void storeWordAt(std::vector<symbols::DataWord>& words, int wordCount, int offsetBytes,
        const symbols::DataWord& operand, int storeSizeBytes) {
    if (offsetBytes < 0 || storeSizeBytes <= 0) {
        return;
    }
    const int wi = type::object_abi::wordIndexAt(offsetBytes);
    if (wi < 0 || wi >= wordCount) {
        return;
    }
    if (storeSizeBytes >= type::object_abi::MACHINE_WORD_SIZE
            || std::holds_alternative<symbols::AddressInit>(operand)) {
        words[static_cast<std::size_t>(wi)] = operand;
        return;
    }
    unsigned long long wordVal = 0;
    if (const auto* existing = std::get_if<symbols::ConstantInit>(&words[static_cast<std::size_t>(wi)])) {
        wordVal = static_cast<unsigned long long>(existing->value);
    }
    const unsigned long long v = static_cast<unsigned long long>(
            std::get<symbols::ConstantInit>(operand).value);
    const int lane = offsetBytes % type::object_abi::MACHINE_WORD_SIZE;
    const int bits = storeSizeBytes * 8;
    const unsigned long long mask = bits >= 64 ? ~0ull : ((1ull << bits) - 1ull);
    wordVal &= ~(mask << (lane * 8));
    wordVal |= (v & mask) << (lane * 8);
    words[static_cast<std::size_t>(wi)] = symbols::ConstantInit { static_cast<long>(wordVal) };
}

struct PackedCharArray {
    std::vector<unsigned char> bytes;
    int size { 0 };
};

// false = not a char[] string. true + empty optional = excess (already reported).
// true + value = decoded, NUL-truncated to size when needed.
bool tryPackCharArrayString(const type::Type& arrayType, ast::Expression* value,
        AggregateInitSink& sink, std::optional<PackedCharArray>& out) {
    out.reset();
    ast::StringLiteralExpression* strLit = asCharArrayStringLiteral(value);
    if (!strLit || !isCharArrayType(arrayType)) {
        return false;
    }
    PackedCharArray packed;
    packed.bytes = util::decodeStringLiteralBytes(strLit->getValue());
    packed.size = arrayType.getArraySize();
    if (packed.size > 0 && static_cast<int>(packed.bytes.size()) > packed.size
            && static_cast<int>(packed.bytes.size()) - 1 > packed.size) {
        sink.error("excess elements in array initializer");
        return true;
    }
    if (packed.size > 0 && static_cast<int>(packed.bytes.size()) > packed.size) {
        packed.bytes.resize(static_cast<std::size_t>(packed.size));
    }
    out = std::move(packed);
    return true;
}

} // namespace

FieldPlanSink::FieldPlanSink(AggregateInitHost& h, SymbolTable& st, translation_unit::Context ctx,
        std::vector<symbols::StructFieldInit>& p)
        : host { h }, symbolTable { st }, context { std::move(ctx) }, plan { p } {
}

bool FieldPlanSink::ok() const {
    return !failed;
}

void FieldPlanSink::error(const std::string& message) {
    failed = true;
    host.error(message, context);
}

void FieldPlanSink::onUnwritten(int offsetBytes, const type::Type& t) {
    if (t.isArray() && t.getArraySize() <= 0) {
        error(kIncompleteArrayInitMsg);
        return;
    }
    const int size = t.getSize();
    if (size <= 0) {
        return;
    }
    symbols::ZeroSpanInit z;
    z.offsetBytes = offsetBytes;
    z.zeroSpanBytes = size;
    z.temps.source = std::make_unique<symbols::ValueEntry>(
            symbolTable.createTemporarySymbol(type::signedLong()));
    z.temps.address = std::make_unique<symbols::ValueEntry>(
            symbolTable.createTemporarySymbol(type::pointer(type::signedLong())));
    plan.push_back(std::move(z));
}

bool FieldPlanSink::placeStringArray(int offsetBytes, const type::Type& arrayType,
        ast::Expression* value) {
    std::optional<PackedCharArray> packed;
    if (!tryPackCharArrayString(arrayType, value, *this, packed)) {
        return false;
    }
    if (!packed) {
        return true;
    }
    symbols::StringBytesInit row;
    row.offsetBytes = offsetBytes;
    row.sizeBytes = packed->size;
    row.bytes = std::move(packed->bytes);
    row.temps.source = std::make_unique<symbols::ValueEntry>(
            symbolTable.createTemporarySymbol(type::signedLong()));
    row.temps.address = std::make_unique<symbols::ValueEntry>(
            symbolTable.createTemporarySymbol(type::pointer(type::signedCharacter())));
    plan.push_back(std::move(row));
    return true;
}

void FieldPlanSink::placeScalar(int offsetBytes, const type::Type& storeType, ast::Expression* value,
        std::optional<type::BitField> bits) {
    auto makeTemps = [&](const type::Type& srcTy, const type::Type& addrPointee) {
        symbols::FieldInitTemps temps;
        temps.source = std::make_unique<symbols::ValueEntry>(
                symbolTable.createTemporarySymbol(srcTy));
        temps.address = std::make_unique<symbols::ValueEntry>(
                symbolTable.createTemporarySymbol(type::pointer(addrPointee)));
        return temps;
    };
    if (value && value->hasResult(host.annotations)) {
        const type::Type src = value->valueType(host.annotations);
        if (storeType.isPointer() && src.isArray()) {
            symbols::AddressOfStoreInit row;
            row.offsetBytes = offsetBytes;
            row.storeType = storeType;
            row.addressOfOperand = value->result(host.annotations)->getName();
            row.temps = makeTemps(storeType, storeType);
            plan.push_back(std::move(row));
            return;
        }
        if (!type::productAssignFrom(storeType, src)) {
            failed = true;
            host.error(type::productAssignFailureMessage(storeType, src), context);
            return;
        }
        maybeSetConversion(value, storeType, symbolTable, host.annotations);
        symbols::ValueStoreInit row;
        row.offsetBytes = offsetBytes;
        row.storeType = storeType;
        row.bitField = bits;
        if (auto* converted = host.annotations.value(value, symbols::ValueSlot::Conversion)) {
            row.temps.source = std::make_unique<symbols::ValueEntry>(*converted);
        } else {
            row.temps.source = std::make_unique<symbols::ValueEntry>(*value->result(host.annotations));
        }
        row.temps.address = std::make_unique<symbols::ValueEntry>(
                symbolTable.createTemporarySymbol(type::pointer(storeType)));
        plan.push_back(std::move(row));
        return;
    }
    long v = 0;
    if (value && value->evaluateConstant(v)) {
        symbols::ConstantStoreInit row;
        row.offsetBytes = offsetBytes;
        row.storeType = storeType;
        row.bitField = bits;
        row.constantValue = std::to_string(type::convertScalarConstant(storeType, v));
        row.temps = makeTemps(storeType, storeType);
        plan.push_back(std::move(row));
        return;
    }
    if (bits) {
        symbols::ConstantStoreInit row;
        row.offsetBytes = offsetBytes;
        row.storeType = storeType;
        row.bitField = bits;
        row.constantValue = "0";
        row.temps = makeTemps(storeType, storeType);
        plan.push_back(std::move(row));
        return;
    }
    const int size = storeType.getSize();
    symbols::ZeroSpanInit z;
    z.offsetBytes = offsetBytes;
    z.zeroSpanBytes = size > 0 ? size : 1;
    z.temps.source = std::make_unique<symbols::ValueEntry>(
            symbolTable.createTemporarySymbol(type::signedLong()));
    z.temps.address = std::make_unique<symbols::ValueEntry>(
            symbolTable.createTemporarySymbol(type::pointer(type::signedLong())));
    plan.push_back(std::move(z));
}

bool FieldPlanSink::placeAggregateCopy(int offsetBytes, const type::Type& storeType,
        ast::Expression* value) {
    if (!value || !value->hasResult(host.annotations)) {
        return false;
    }
    const type::Type src = value->valueType(host.annotations);
    if (!src.isAggregate()) {
        return false;
    }
    if (!type::productAssignFrom(storeType, src)) {
        failed = true;
        host.error(type::productAssignFailureMessage(storeType, src), context);
        return true;
    }
    symbols::ValueStoreInit row;
    row.offsetBytes = offsetBytes;
    row.storeType = storeType;
    row.temps.source = std::make_unique<symbols::ValueEntry>(*value->result(host.annotations));
    row.temps.address = std::make_unique<symbols::ValueEntry>(
            symbolTable.createTemporarySymbol(type::pointer(storeType)));
    plan.push_back(std::move(row));
    return true;
}

DataWordSink::DataWordSink(AggregateInitHost& h, translation_unit::Context ctx,
        std::vector<symbols::DataWord>& w, int wc)
        : host { h }, context { std::move(ctx) }, words { w }, wordCount { wc } {
}

bool DataWordSink::ok() const {
    return !failed;
}

void DataWordSink::error(const std::string& message) {
    failed = true;
    host.error(message, context);
}

void DataWordSink::onUnwritten(int offsetBytes, const type::Type& t) {
    forEachInitStorageUnit(t, offsetBytes,
            [&](int off, const type::Type& storeType) {
                storeWordAt(words, wordCount, off, symbols::ConstantInit { 0 }, storeType.getSize());
            },
            [&]() { error(kIncompleteArrayInitMsg); });
}

bool DataWordSink::placeStringArray(int offsetBytes, const type::Type& arrayType,
        ast::Expression* value) {
    std::optional<PackedCharArray> packed;
    if (!tryPackCharArrayString(arrayType, value, *this, packed)) {
        return false;
    }
    if (!packed) {
        return true;
    }
    for (int i = 0; i < packed->size; ++i) {
        const unsigned char ch = (i < static_cast<int>(packed->bytes.size()))
                ? packed->bytes[static_cast<std::size_t>(i)] : 0;
        storeWordAt(words, wordCount, offsetBytes + i,
                symbols::ConstantInit { static_cast<long>(ch) }, 1);
    }
    return true;
}

void DataWordSink::placeScalar(int offsetBytes, const type::Type& storeType, ast::Expression* value,
        std::optional<type::BitField> bits) {
    if (bits) {
        long v = 0;
        if (value && !value->evaluateConstant(v)) {
            error("global brace initializer is not a constant expression");
            return;
        }
        const auto& bf = *bits;
        const int absBit = offsetBytes * 8 + bf.shift;
        const int wi = type::object_abi::wordIndexAt(absBit / 8);
        if (wi < 0 || wi >= wordCount) {
            return;
        }
        unsigned long long wordVal = 0;
        if (const auto* existing = std::get_if<symbols::ConstantInit>(
                    &words[static_cast<std::size_t>(wi)])) {
            wordVal = static_cast<unsigned long long>(existing->value);
        }
        const int shift = absBit % (type::object_abi::MACHINE_WORD_SIZE * 8);
        const unsigned long long mask = type::bitFieldMask(bf.width);
        wordVal &= ~(mask << shift);
        wordVal |= (static_cast<unsigned long long>(v) & mask) << shift;
        words[static_cast<std::size_t>(wi)] = symbols::ConstantInit { static_cast<long>(wordVal) };
        return;
    }
    if (!value) {
        return;
    }
    symbols::DataWord operand;
    if (tryFoldDataWord(value, storeType, host.annotations, operand)) {
        storeWordAt(words, wordCount, offsetBytes, operand, storeType.getSize());
        return;
    }
    error("global brace initializer is not a constant expression");
}

} // namespace semantic_analyzer
