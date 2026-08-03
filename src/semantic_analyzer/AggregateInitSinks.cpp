#include "AggregateInitSinks.h"

#include "ConstantAddress.h"

#include "ast/StringLiteralExpression.h"
#include "types/ObjectAbi.h"
#include "types/TypeQuery.h"
#include "util/StringLiteralDecode.h"

#include <functional>

namespace semantic_analyzer {

namespace {

bool parseWord(const std::string& s, unsigned long long& out) {
    if (s.empty()) {
        return false;
    }
    try {
        std::size_t idx = 0;
        if (s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
            out = std::stoull(s, &idx, 16);
        } else {
            long long signedVal = std::stoll(s, &idx, 10);
            out = static_cast<unsigned long long>(signedVal);
        }
        return idx == s.size();
    } catch (...) {
        return false;
    }
}

void storeWordAt(std::vector<std::string>& words, int wordCount, int offsetBytes,
        const std::string& operand, int storeSizeBytes) {
    if (offsetBytes < 0 || storeSizeBytes <= 0) {
        return;
    }
    const int wi = type::object_abi::wordIndexAt(offsetBytes);
    if (wi < 0 || wi >= wordCount) {
        return;
    }
    if (storeSizeBytes >= type::object_abi::MACHINE_WORD_SIZE) {
        words[static_cast<std::size_t>(wi)] = operand;
        return;
    }
    unsigned long long wordVal = 0;
    parseWord(words[static_cast<std::size_t>(wi)], wordVal);
    unsigned long long v = 0;
    if (!parseWord(operand, v)) {
        words[static_cast<std::size_t>(wi)] = operand;
        return;
    }
    const int lane = offsetBytes % type::object_abi::MACHINE_WORD_SIZE;
    const int bits = storeSizeBytes * 8;
    const unsigned long long mask = bits >= 64 ? ~0ull : ((1ull << bits) - 1ull);
    wordVal &= ~(mask << (lane * 8));
    wordVal |= (v & mask) << (lane * 8);
    words[static_cast<std::size_t>(wi)] = formatDataWord(wordVal);
}

void placeStringBytes(int offsetBytes, const type::Type& arrayType, ast::Expression* value,
        std::function<void(int, const type::Type&, const std::string&)> storeByte) {
    auto* strLit = dynamic_cast<ast::StringLiteralExpression*>(value);
    if (!strLit || !isCharArrayType(arrayType)) {
        return;
    }
    const auto bytes = util::decodeStringLiteralBytes(strLit->getValue());
    const int n = arrayType.getArraySize();
    for (int i = 0; i < n; ++i) {
        const unsigned char ch = (i < static_cast<int>(bytes.size()))
                ? bytes[static_cast<std::size_t>(i)] : 0;
        storeByte(offsetBytes + i, type::signedCharacter(),
                std::to_string(static_cast<unsigned>(ch)));
    }
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
    // One plan entry zeros the whole span (includes inter-member padding).
    symbols::StructFieldInit field;
    field.offsetBytes = offsetBytes;
    field.zeroSpanBytes = size;
    field.source = std::make_unique<symbols::ValueEntry>(
            symbolTable.createTemporarySymbol(type::signedLong()));
    field.address = std::make_unique<symbols::ValueEntry>(
            symbolTable.createTemporarySymbol(type::pointer(type::signedLong())));
    plan.push_back(std::move(field));
}

bool FieldPlanSink::placeStringArray(int offsetBytes, const type::Type& arrayType,
        ast::Expression* value) {
    if (!isCharArrayType(arrayType) || !value
            || !dynamic_cast<ast::StringLiteralExpression*>(value)) {
        return false;
    }
    placeStringBytes(offsetBytes, arrayType, value,
            [&](int off, const type::Type& byteTy, const std::string& cval) {
                symbols::StructFieldInit field;
                field.offsetBytes = off;
                field.constantValue = cval;
                field.source = std::make_unique<symbols::ValueEntry>(
                        symbolTable.createTemporarySymbol(byteTy));
                field.address = std::make_unique<symbols::ValueEntry>(
                        symbolTable.createTemporarySymbol(type::pointer(byteTy)));
                plan.push_back(std::move(field));
            });
    return true;
}

void FieldPlanSink::placeScalar(int offsetBytes, const type::Type& storeType, ast::Expression* value) {
    symbols::StructFieldInit field;
    field.offsetBytes = offsetBytes;
    field.address = std::make_unique<symbols::ValueEntry>(
            symbolTable.createTemporarySymbol(type::pointer(storeType)));
    if (value && value->hasResult(host.annotations)) {
        const type::Type src = value->valueType(host.annotations);
        if (storeType.isPointer() && src.isArray()) {
            field.addressOfOperand = value->result(host.annotations)->getName();
            field.source = std::make_unique<symbols::ValueEntry>(
                    symbolTable.createTemporarySymbol(storeType));
            plan.push_back(std::move(field));
            return;
        }
        if (!type::productAssignFrom(storeType, src)) {
            failed = true;
            host.error(type::productAssignFailureMessage(storeType, src), context);
            return;
        }
        field.source = std::make_unique<symbols::ValueEntry>(*value->result(host.annotations));
    } else {
        long v = 0;
        if (value && value->evaluateConstant(v)) {
            field.constantValue = std::to_string(v);
            field.source = std::make_unique<symbols::ValueEntry>(
                    symbolTable.createTemporarySymbol(storeType));
        } else {
            // Scalar zero uses the same span model as full-object padding zero.
            const int size = storeType.getSize();
            field.zeroSpanBytes = size > 0 ? size : 1;
            field.source = std::make_unique<symbols::ValueEntry>(
                    symbolTable.createTemporarySymbol(type::signedLong()));
            field.address = std::make_unique<symbols::ValueEntry>(
                    symbolTable.createTemporarySymbol(type::pointer(type::signedLong())));
            plan.push_back(std::move(field));
            return;
        }
    }
    plan.push_back(std::move(field));
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
        return true; // handled (as error); walk must not peel into first subobject
    }
    symbols::StructFieldInit field;
    field.offsetBytes = offsetBytes;
    field.source = std::make_unique<symbols::ValueEntry>(*value->result(host.annotations));
    field.address = std::make_unique<symbols::ValueEntry>(
            symbolTable.createTemporarySymbol(type::pointer(storeType)));
    plan.push_back(std::move(field));
    return true;
}

DataWordSink::DataWordSink(AggregateInitHost& h, translation_unit::Context ctx,
        std::vector<std::string>& w, int wc)
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
    // Prefill is "0", but last-wins designators may re-enter a non-zero region.
    forEachInitStorageUnit(t, offsetBytes,
            [&](int off, const type::Type& storeType) {
                storeWordAt(words, wordCount, off, "0", storeType.getSize());
            },
            [&]() { error(kIncompleteArrayInitMsg); });
}

bool DataWordSink::placeStringArray(int offsetBytes, const type::Type& arrayType,
        ast::Expression* value) {
    if (!isCharArrayType(arrayType) || !value
            || !dynamic_cast<ast::StringLiteralExpression*>(value)) {
        return false;
    }
    placeStringBytes(offsetBytes, arrayType, value,
            [&](int off, const type::Type& byteTy, const std::string& cval) {
                storeWordAt(words, wordCount, off, cval, byteTy.getSize());
            });
    return true;
}

void DataWordSink::placeScalar(int offsetBytes, const type::Type& storeType, ast::Expression* value) {
    if (!value) {
        return;
    }
    std::string operand;
    if (tryFoldDataOperand(value, storeType, host.annotations, operand)) {
        storeWordAt(words, wordCount, offsetBytes, operand, storeType.getSize());
        return;
    }
    error("global brace initializer is not a constant expression");
}

} // namespace semantic_analyzer
