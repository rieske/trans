#include "AggregateInitSinks.h"

#include "SemanticAnalysisVisitorInternal.h"

#include "types/ObjectAbi.h"
#include "types/TypeQuery.h"
#include "util/ImmediateFormat.h"

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

void FieldPlanSink::onUnwritten(int offsetBytes, const type::Type& t) {
    forEachInitStorageUnit(t, offsetBytes,
            [&](int off, const type::Type& storeType) {
                symbols::StructFieldInit field;
                field.offsetBytes = off;
                auto addr = symbolTable.createTemporarySymbol(type::pointer(storeType));
                field.addressName = addr.getName();
                auto zero = symbolTable.createTemporarySymbol(storeType);
                field.zeroInitialize = true;
                field.sourceName = zero.getName();
                plan.push_back(std::move(field));
            },
            [&]() {
                error("array brace initializers for incomplete arrays are not implemented");
            });
}

void FieldPlanSink::placeScalar(int offsetBytes, const type::Type& storeType, ast::Expression* value) {
    symbols::StructFieldInit field;
    field.offsetBytes = offsetBytes;
    auto addr = symbolTable.createTemporarySymbol(type::pointer(storeType));
    field.addressName = addr.getName();
    if (value && value->hasResultSymbol(annotations)) {
        const type::Type src = assignSourceType(*value, storeType, annotations);
        if (!storeType.canAssignFrom(src)) {
            failed = true;
        }
        visitor.typeCheck(src, storeType, context);
        field.zeroInitialize = false;
        field.sourceName = value->getResultSymbol(annotations)->getName();
    } else {
        auto zero = symbolTable.createTemporarySymbol(storeType);
        field.zeroInitialize = true;
        field.sourceName = zero.getName();
    }
    plan.push_back(std::move(field));
}

DataWordSink::DataWordSink(SemanticAnalysisVisitor& v, translation_unit::Context ctx,
        std::vector<std::string>& w, int wc)
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

std::string formatWord(unsigned long long v) {
    return util::wordImmediate(v);
}

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

void storeWordAt(std::vector<std::string>& words, int wordCount, int offsetBytes, long value,
        int storeSizeBytes) {
    if (offsetBytes < 0 || storeSizeBytes <= 0) {
        return;
    }
    const int wi = type::object_abi::wordIndexAt(offsetBytes);
    if (wi < 0 || wi >= wordCount) {
        return;
    }
    if (storeSizeBytes >= type::object_abi::MACHINE_WORD_SIZE) {
        words[static_cast<std::size_t>(wi)] = formatWord(static_cast<unsigned long long>(value));
        return;
    }
    unsigned long long wordVal = 0;
    parseWord(words[static_cast<std::size_t>(wi)], wordVal);
    const int lane = offsetBytes % type::object_abi::MACHINE_WORD_SIZE;
    const int bits = storeSizeBytes * 8;
    const unsigned long long mask = bits >= 64 ? ~0ull : ((1ull << bits) - 1ull);
    wordVal &= ~(mask << (lane * 8));
    wordVal |= (static_cast<unsigned long long>(value) & mask) << (lane * 8);
    words[static_cast<std::size_t>(wi)] = formatWord(wordVal);
}

} // namespace

void DataWordSink::onUnwritten(int offsetBytes, const type::Type& t) {
    forEachInitStorageUnit(t, offsetBytes,
            [&](int off, const type::Type& storeType) {
                storeWordAt(words, wordCount, off, 0, storeType.getSize());
            },
            [&]() {
                error("array brace initializers for incomplete arrays are not implemented");
            });
}

void DataWordSink::placeScalar(int offsetBytes, const type::Type& storeType, ast::Expression* value) {
    if (!value) {
        return;
    }
    long v = 0;
    if (!value->evaluateConstant(v)) {
        error("global brace initializer is not a constant expression");
        return;
    }
    storeWordAt(words, wordCount, offsetBytes, v, storeType.getSize());
}

} // namespace semantic_analyzer
