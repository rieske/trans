#include "AggregateInitSinks.h"

#include "CharArrayStringInit.h"
#include "ConstantAddress.h"
#include "Conversion.h"
#include "InitializerLowering.h"
#include "ProductAssign.h"

#include "types/IntegerConstant.h"
#include "types/ObjectAbi.h"

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

void storeInitWords(std::vector<symbols::DataWord>& words, int wordCount, int offsetBytes,
        int storeSizeBytes, const symbols::GlobalInitializer& init) {
    if (const auto* multi = std::get_if<symbols::MultiWordInit>(&init)) {
        int off = offsetBytes;
        for (const auto& word : multi->words) {
            storeWordAt(words, wordCount, off, word, type::object_abi::MACHINE_WORD_SIZE);
            off += type::object_abi::MACHINE_WORD_SIZE;
        }
        return;
    }
    if (const auto* constant = std::get_if<symbols::ConstantInit>(&init)) {
        storeWordAt(words, wordCount, offsetBytes, *constant, storeSizeBytes);
        return;
    }
    storeWordAt(words, wordCount, offsetBytes, std::get<symbols::AddressInit>(init), storeSizeBytes);
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
    std::string err;
    auto bytes = charArrayBytesFromString(arrayType, value, &err);
    if (!err.empty()) {
        sink.error(err);
        return true;
    }
    if (!bytes) {
        return false;
    }
    PackedCharArray packed;
    packed.bytes = std::move(*bytes);
    packed.size = arrayType.isIncompleteArray() ? static_cast<int>(packed.bytes.size())
                                                : arrayType.getArraySize();
    if (packed.size < static_cast<int>(packed.bytes.size())) {
        packed.size = static_cast<int>(packed.bytes.size());
    }
    out = std::move(packed);
    return true;
}

symbols::FieldInitTemps makeFieldTemps(SymbolTable& symbolTable, const type::Type& srcTy,
        const type::Type& addrPointee) {
    symbols::FieldInitTemps temps;
    temps.source = std::make_unique<symbols::ValueEntry>(
            symbolTable.createTemporarySymbol(srcTy));
    temps.address = std::make_unique<symbols::ValueEntry>(
            symbolTable.createTemporarySymbol(type::pointer(addrPointee)));
    return temps;
}

} // namespace

FieldPlanSink::FieldPlanSink(AggregateInitHost& h, SymbolTable& st, translation_unit::Context ctx,
        std::vector<symbols::FieldInit>& p)
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
    const int size = t.getSize();
    if (size <= 0) {
        return;
    }
    plan.push_back(symbols::fieldZeroSpan(offsetBytes, size,
            makeFieldTemps(symbolTable, type::signedLong(), type::signedLong())));
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
    plan.push_back(symbols::fieldStringBytes(offsetBytes, packed->size, std::move(packed->bytes),
            makeFieldTemps(symbolTable, type::signedLong(), type::signedCharacter())));
    return true;
}

void FieldPlanSink::placeScalar(int offsetBytes, const type::Type& storeType, ast::Expression* value,
        std::optional<type::BitField> bits) {
    if (value && value->hasResultSymbol(host.annotations)) {
        const type::Type src = value->valueType(host.annotations);
        if (storeType.isPointer() && src.isArray()) {
            plan.push_back(symbols::fieldAddressOf(offsetBytes, storeType,
                    value->getResultSymbol(host.annotations)->getName(),
                    makeFieldTemps(symbolTable, storeType, storeType)));
            return;
        }
        if (!reportProductAssign(host.error, storeType, src, context, value)) {
            failed = true;
            return;
        }
        maybeSetConversion(value, storeType, symbolTable, host.annotations);
        symbols::FieldInitTemps temps;
        if (auto* converted = host.annotations.value(value, symbols::ValueSlot::Conversion)) {
            temps.source = std::make_unique<symbols::ValueEntry>(*converted);
        } else {
            temps.source = std::make_unique<symbols::ValueEntry>(*value->getResultSymbol(host.annotations));
        }
        temps.address = std::make_unique<symbols::ValueEntry>(
                symbolTable.createTemporarySymbol(type::pointer(storeType)));
        plan.push_back(symbols::fieldValue(offsetBytes, storeType, std::move(temps), bits));
        return;
    }
    long v = 0;
    if (value && value->foldToHostLong(v)) {
        plan.push_back(symbols::fieldConstant(offsetBytes, storeType,
                std::to_string(type::toHostLong(type::convert(type::fromHostLong(v), storeType))),
                makeFieldTemps(symbolTable, storeType, storeType), bits));
        return;
    }
    if (bits) {
        plan.push_back(symbols::fieldConstant(offsetBytes, storeType, "0",
                makeFieldTemps(symbolTable, storeType, storeType), bits));
        return;
    }
    const int size = storeType.getSize();
    plan.push_back(symbols::fieldZeroSpan(offsetBytes, size > 0 ? size : 1,
            makeFieldTemps(symbolTable, type::signedLong(), type::signedLong())));
}

bool FieldPlanSink::placeAggregateCopy(int offsetBytes, const type::Type& storeType,
        ast::Expression* value) {
    if (!value || !value->hasResultSymbol(host.annotations)) {
        return false;
    }
    const type::Type src = value->valueType(host.annotations);
    if (!src.isAggregate()) {
        return false;
    }
    if (!reportProductAssign(host.error, storeType, src, context, value)) {
        failed = true;
        return true;
    }
    symbols::FieldInitTemps temps;
    temps.source = std::make_unique<symbols::ValueEntry>(*value->getResultSymbol(host.annotations));
    temps.address = std::make_unique<symbols::ValueEntry>(
            symbolTable.createTemporarySymbol(type::pointer(storeType)));
    plan.push_back(symbols::fieldValue(offsetBytes, storeType, std::move(temps)));
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
        if (value && !value->foldToHostLong(v)) {
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
    symbols::GlobalInitializer init;
    if (tryFoldGlobalInit(value, storeType, host.annotations, init)) {
        storeInitWords(words, wordCount, offsetBytes, storeType.getSize(), init);
        return;
    }
    error("global brace initializer is not a constant expression");
}

} // namespace semantic_analyzer
