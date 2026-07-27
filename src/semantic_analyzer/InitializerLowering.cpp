#include "SemanticAnalysisVisitorInternal.h"

#include <functional>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "ast/InitializerListExpression.h"
#include "types/ObjectAbi.h"
#include "types/TypeQuery.h"

namespace semantic_analyzer {

namespace {

// Walk designator steps from destType at baseOffset -> place type and offset.
// firstTopLevelIndex: for positional resume after designation (member or array index).
bool resolveDesignator(const type::Type& destType, int baseOffset,
        const std::vector<ast::DesignatorStep>& steps, type::Type& outType, int& outOffset,
        int& firstTopLevelIndex, std::string& error) {
    if (steps.empty()) {
        error = "empty designator";
        return false;
    }
    type::Type cur = destType;
    int offset = baseOffset;
    firstTopLevelIndex = -1;

    for (std::size_t si = 0; si < steps.size(); ++si) {
        const auto& step = steps[si];
        if (step.kind == ast::DesignatorStep::Kind::Member) {
            if (!cur.isRecord()) {
                error = "designated initializer member not found";
                return false;
            }
            int mi = -1;
            for (int i = 0; i < cur.memberCount(); ++i) {
                std::string n;
                type::Type t = type::voidType();
                int off = 0;
                if (!cur.memberAt(i, n, t, off)) {
                    break;
                }
                if (n == step.memberName) {
                    mi = i;
                    offset += off;
                    cur = t;
                    break;
                }
            }
            if (mi < 0) {
                error = "designated initializer member not found";
                return false;
            }
            if (si == 0) {
                firstTopLevelIndex = mi;
            }
        } else {
            if (!step.index) {
                error = "designated array index is not a constant expression";
                return false;
            }
            const long idx = *step.index;
            if (cur.isArray()) {
                const int n = cur.getArraySize();
                if (n <= 0) {
                    error = "array brace initializers for incomplete arrays are not implemented";
                    return false;
                }
                if (idx < 0 || idx >= n) {
                    error = "designated initializer index out of range";
                    return false;
                }
                offset += static_cast<int>(idx) * cur.getElementStride();
                cur = cur.getElementType();
                if (si == 0) {
                    firstTopLevelIndex = static_cast<int>(idx);
                }
            } else if (cur.isStructure()) {
                // C allows [n] as an alternate form for the nth member of a struct.
                if (idx < 0 || idx >= cur.memberCount()) {
                    error = "designated initializer index out of range";
                    return false;
                }
                std::string n;
                type::Type t = type::voidType();
                int off = 0;
                if (!cur.memberAt(static_cast<int>(idx), n, t, off)) {
                    error = "designated initializer index out of range";
                    return false;
                }
                offset += off;
                cur = t;
                if (si == 0) {
                    firstTopLevelIndex = static_cast<int>(idx);
                }
            } else {
                error = "designated initializer index out of range";
                return false;
            }
        }
    }
    outType = cur;
    outOffset = offset;
    return true;
}

// Policy sink for one placement pass over an aggregate initializer.
struct AggregateInitSink {
    virtual ~AggregateInitSink() = default;
    virtual void placeScalar(int offsetBytes, const type::Type& storeType, ast::Expression* value) = 0;
    virtual void onUnwritten(int offsetBytes, const type::Type& t) = 0;
    virtual void error(const std::string& message) = 0;
    virtual bool ok() const = 0;
};

void walkAggregateInit(const type::Type& targetType, const ast::InitializerListExpression* list,
        int baseOffset, AggregateInitSink& sink);

void placeAt(const type::Type& placeType, int offsetBytes, ast::Expression* value, AggregateInitSink& sink) {
    if (!sink.ok()) {
        return;
    }
    if (auto* nestedList = value ? dynamic_cast<ast::InitializerListExpression*>(value) : nullptr) {
        if (placeType.isAggregate() || placeType.isArray()) {
            walkAggregateInit(placeType, nestedList, offsetBytes, sink);
            return;
        }
    }
    if (placeType.isAggregate() || placeType.isArray()) {
        if (value) {
            // Non-brace value into aggregate without current-object stream: reject.
            sink.error("aggregate member initializer requires nested braces (not implemented)");
            return;
        }
        sink.onUnwritten(offsetBytes, placeType);
        return;
    }
    // Scalar (possibly braced { e }).
    if (value) {
        auto* nested = dynamic_cast<ast::InitializerListExpression*>(value);
        while (nested) {
            if (nested->getElements().size() > 1) {
                sink.error("excess elements in scalar initializer");
                return;
            }
            if (nested->getElements().empty() || !nested->getElements().front().value) {
                sink.onUnwritten(offsetBytes, placeType);
                return;
            }
            value = nested->getElements().front().value.get();
            nested = dynamic_cast<ast::InitializerListExpression*>(value);
        }
        sink.placeScalar(offsetBytes, placeType, value);
    } else {
        sink.onUnwritten(offsetBytes, placeType);
    }
}

// Fill aggregate from flat element stream (C current-object). Returns new element index.
std::size_t fillFromStream(const type::Type& destType, int baseOffset,
        const std::vector<ast::InitializerElement>& elements, std::size_t ei, AggregateInitSink& sink) {
    if (!sink.ok()) {
        return ei;
    }
    if (destType.isUnion()) {
        if (ei >= elements.size() || !elements[ei].value || elements[ei].isDesignated()) {
            sink.onUnwritten(baseOffset, destType);
            return ei;
        }
        auto* nested = dynamic_cast<ast::InitializerListExpression*>(elements[ei].value.get());
        if (nested) {
            walkAggregateInit(destType, nested, baseOffset, sink);
            return ei + 1;
        }
        if (destType.memberCount() < 1) {
            return ei + 1;
        }
        std::string name;
        type::Type first = type::voidType();
        int off = 0;
        if (!destType.memberAt(0, name, first, off)) {
            return ei + 1;
        }
        if ((first.isStructure() || first.isArray() || first.isUnion()) && !nested) {
            return fillFromStream(first, baseOffset + off, elements, ei, sink);
        }
        placeAt(first, baseOffset + off, elements[ei].value.get(), sink);
        return ei + 1;
    }
    if (destType.isStructure()) {
        for (int mi = 0; mi < destType.memberCount(); ++mi) {
            std::string name;
            type::Type memberType = type::voidType();
            int offset = 0;
            if (!destType.memberAt(mi, name, memberType, offset)) {
                break;
            }
            if (ei >= elements.size() || elements[ei].isDesignated()) {
                sink.onUnwritten(baseOffset + offset, memberType);
                continue;
            }
            auto* nested = dynamic_cast<ast::InitializerListExpression*>(elements[ei].value.get());
            if (nested && (memberType.isStructure() || memberType.isArray() || memberType.isUnion())) {
                walkAggregateInit(memberType, nested, baseOffset + offset, sink);
                ++ei;
                continue;
            }
            if ((memberType.isStructure() || memberType.isArray() || memberType.isUnion()) && !nested) {
                ei = fillFromStream(memberType, baseOffset + offset, elements, ei, sink);
                continue;
            }
            placeAt(memberType, baseOffset + offset, elements[ei].value.get(), sink);
            ++ei;
        }
        return ei;
    }
    if (destType.isArray()) {
        const int n = destType.getArraySize();
        if (n <= 0) {
            sink.error("array brace initializers for incomplete arrays are not implemented");
            return ei;
        }
        const int stride = destType.getElementStride();
        const type::Type elem = destType.getElementType();
        for (int i = 0; i < n; ++i) {
            if (ei >= elements.size() || elements[ei].isDesignated()) {
                sink.onUnwritten(baseOffset + i * stride, elem);
                continue;
            }
            auto* nested = dynamic_cast<ast::InitializerListExpression*>(elements[ei].value.get());
            if (nested && (elem.isStructure() || elem.isArray() || elem.isUnion())) {
                walkAggregateInit(elem, nested, baseOffset + i * stride, sink);
                ++ei;
                continue;
            }
            if ((elem.isStructure() || elem.isArray() || elem.isUnion()) && !nested) {
                ei = fillFromStream(elem, baseOffset + i * stride, elements, ei, sink);
                continue;
            }
            placeAt(elem, baseOffset + i * stride, elements[ei].value.get(), sink);
            ++ei;
        }
        return ei;
    }
    if (ei >= elements.size() || elements[ei].isDesignated()) {
        sink.onUnwritten(baseOffset, destType);
        return ei;
    }
    placeAt(destType, baseOffset, elements[ei].value.get(), sink);
    return ei + 1;
}

void walkAggregateInit(const type::Type& targetType, const ast::InitializerListExpression* list,
        int baseOffset, AggregateInitSink& sink) {
    if (!sink.ok() || !list) {
        return;
    }
    // Mutable copy so we can fold designator indexes in place.
    std::vector<ast::InitializerElement> elements;
    // Cannot copy InitializerElement (unique_ptr). Walk via const list + fold on the fly.
    const auto& src = list->getElements();

    auto foldSteps = [&](const ast::InitializerElement& el, std::vector<ast::DesignatorStep>& stepsOut) -> bool {
        // Clone steps shallowly: share expression raw via re-fold only.
        stepsOut.clear();
        for (const auto& s : el.designator) {
            ast::DesignatorStep copy;
            copy.kind = s.kind;
            copy.memberName = s.memberName;
            copy.index = s.index;
            // indexExpression is non-owning for fold attempt: evaluate via const.
            if (!copy.index && s.indexExpression) {
                long v = 0;
                if (s.indexExpression->evaluateConstant(v)) {
                    copy.index = v;
                } else {
                    sink.error("designated array index is not a constant expression");
                    return false;
                }
            } else if (copy.kind == ast::DesignatorStep::Kind::Index && !copy.index) {
                sink.error("designated array index is not a constant expression");
                return false;
            }
            stepsOut.push_back(std::move(copy));
        }
        return true;
    };

    if (targetType.isUnion()) {
        if (src.size() > 1) {
            sink.error("excess elements in union initializer");
            // continue with first
        }
        if (src.empty() || !src.front().value) {
            sink.onUnwritten(baseOffset, targetType);
            return;
        }
        const auto& el = src.front();
        if (el.isDesignated()) {
            std::vector<ast::DesignatorStep> steps;
            if (!foldSteps(el, steps)) {
                return;
            }
            type::Type placeType = type::voidType();
            int placeOff = 0;
            int firstIdx = -1;
            std::string err;
            if (!resolveDesignator(targetType, baseOffset, steps, placeType, placeOff, firstIdx, err)) {
                sink.error(err);
                return;
            }
            placeAt(placeType, placeOff, el.value.get(), sink);
            return;
        }
        if (targetType.memberCount() < 1) {
            return;
        }
        std::string name;
        type::Type first = type::voidType();
        int off = 0;
        if (!targetType.memberAt(0, name, first, off)) {
            return;
        }
        placeAt(first, baseOffset + off, el.value.get(), sink);
        return;
    }

    if (targetType.isStructure()) {
        const int nMembers = targetType.memberCount();
        std::vector<bool> written(static_cast<std::size_t>(nMembers), false);
        std::size_t ei = 0;
        int positional = 0;

        auto markWritten = [&](int mi) {
            if (mi >= 0 && mi < nMembers) {
                written[static_cast<std::size_t>(mi)] = true;
            }
        };

        while (ei < src.size() && sink.ok()) {
            const auto& el = src[ei];
            if (!el.value) {
                ++ei;
                continue;
            }
            if (el.isDesignated()) {
                std::vector<ast::DesignatorStep> steps;
                if (!foldSteps(el, steps)) {
                    return;
                }
                type::Type placeType = type::voidType();
                int placeOff = 0;
                int firstIdx = -1;
                std::string err;
                if (!resolveDesignator(targetType, baseOffset, steps, placeType, placeOff, firstIdx, err)) {
                    sink.error(err);
                    ++ei;
                    continue;
                }
                // Nested path: zero whole first-level member once before leaf stores.
                if (firstIdx >= 0 && steps.size() > 1 && !written[static_cast<std::size_t>(firstIdx)]) {
                    std::string name;
                    type::Type mt = type::voidType();
                    int off = 0;
                    if (targetType.memberAt(firstIdx, name, mt, off)) {
                        sink.onUnwritten(baseOffset + off, mt);
                    }
                }
                markWritten(firstIdx);
                placeAt(placeType, placeOff, el.value.get(), sink);
                if (firstIdx >= 0) {
                    positional = firstIdx + 1;
                }
                ++ei;
                continue;
            }
            if (positional >= nMembers) {
                sink.error("excess elements in structure initializer");
                ++ei;
                continue;
            }
            std::string name;
            type::Type memberType = type::voidType();
            int offset = 0;
            if (!targetType.memberAt(positional, name, memberType, offset)) {
                break;
            }
            auto* nested = dynamic_cast<ast::InitializerListExpression*>(el.value.get());
            if (nested && (memberType.isStructure() || memberType.isArray() || memberType.isUnion())) {
                markWritten(positional);
                walkAggregateInit(memberType, nested, baseOffset + offset, sink);
                ++ei;
                ++positional;
                continue;
            }
            if ((memberType.isStructure() || memberType.isArray() || memberType.isUnion()) && !nested) {
                const std::size_t before = ei;
                ei = fillFromStream(memberType, baseOffset + offset, src, ei, sink);
                // Mark written after fill; if no progress, zero then mark.
                if (ei == before) {
                    sink.onUnwritten(baseOffset + offset, memberType);
                }
                markWritten(positional);
                ++positional;
                continue;
            }
            markWritten(positional);
            placeAt(memberType, baseOffset + offset, el.value.get(), sink);
            ++ei;
            ++positional;
        }
        for (int i = 0; i < nMembers && sink.ok(); ++i) {
            if (!written[static_cast<std::size_t>(i)]) {
                std::string name;
                type::Type memberType = type::voidType();
                int offset = 0;
                if (targetType.memberAt(i, name, memberType, offset)) {
                    sink.onUnwritten(baseOffset + offset, memberType);
                }
            }
        }
        return;
    }

    if (targetType.isArray()) {
        const int n = targetType.getArraySize();
        if (n <= 0) {
            sink.error("array brace initializers for incomplete arrays are not implemented");
            return;
        }
        std::vector<bool> written(static_cast<std::size_t>(n), false);
        std::size_t ei = 0;
        int positional = 0;
        const int stride = targetType.getElementStride();
        const type::Type elem = targetType.getElementType();

        while (ei < src.size() && sink.ok()) {
            const auto& el = src[ei];
            if (!el.value) {
                ++ei;
                continue;
            }
            if (el.isDesignated()) {
                std::vector<ast::DesignatorStep> steps;
                if (!foldSteps(el, steps)) {
                    return;
                }
                type::Type placeType = type::voidType();
                int placeOff = 0;
                int firstIdx = -1;
                std::string err;
                if (!resolveDesignator(targetType, baseOffset, steps, placeType, placeOff, firstIdx, err)) {
                    sink.error(err);
                    ++ei;
                    continue;
                }
                if (firstIdx >= 0 && steps.size() > 1 && !written[static_cast<std::size_t>(firstIdx)]) {
                    sink.onUnwritten(baseOffset + firstIdx * stride, elem);
                }
                if (firstIdx >= 0) {
                    written[static_cast<std::size_t>(firstIdx)] = true;
                    positional = firstIdx + 1;
                }
                placeAt(placeType, placeOff, el.value.get(), sink);
                ++ei;
                continue;
            }
            if (positional >= n) {
                sink.error("excess elements in array initializer");
                ++ei;
                continue;
            }
            auto* nested = dynamic_cast<ast::InitializerListExpression*>(el.value.get());
            if (nested && (elem.isStructure() || elem.isArray() || elem.isUnion())) {
                written[static_cast<std::size_t>(positional)] = true;
                walkAggregateInit(elem, nested, baseOffset + positional * stride, sink);
                ++ei;
                ++positional;
                continue;
            }
            if ((elem.isStructure() || elem.isArray() || elem.isUnion()) && !nested) {
                const std::size_t before = ei;
                ei = fillFromStream(elem, baseOffset + positional * stride, src, ei, sink);
                if (ei == before) {
                    sink.onUnwritten(baseOffset + positional * stride, elem);
                }
                written[static_cast<std::size_t>(positional)] = true;
                ++positional;
                continue;
            }
            written[static_cast<std::size_t>(positional)] = true;
            placeAt(elem, baseOffset + positional * stride, el.value.get(), sink);
            ++ei;
            ++positional;
        }
        for (int i = 0; i < n && sink.ok(); ++i) {
            if (!written[static_cast<std::size_t>(i)]) {
                sink.onUnwritten(baseOffset + i * stride, elem);
            }
        }
        return;
    }
    sink.error("brace initializer for non-aggregate type");
}

// --- Local field-init sink ---

struct FieldPlanSink : AggregateInitSink {
    SemanticAnalysisVisitor& visitor;
    semantic_analyzer::SymbolTable& symbolTable;
    symbols::AnnotationStore& annotations;
    translation_unit::Context context;
    std::vector<symbols::StructFieldInit>& plan;
    bool failed { false };

    FieldPlanSink(SemanticAnalysisVisitor& v, semantic_analyzer::SymbolTable& st,
            symbols::AnnotationStore& ann, translation_unit::Context ctx,
            std::vector<symbols::StructFieldInit>& p)
            : visitor { v }, symbolTable { st }, annotations { ann }, context { std::move(ctx) },
              plan { p } {
    }

    bool ok() const override { return !failed; }

    void error(const std::string& message) override {
        failed = true;
        visitor.semanticError(message, context);
    }

    void emitZero(int offsetBytes, const type::Type& storeType) {
        symbols::StructFieldInit field;
        field.offsetBytes = offsetBytes;
        auto addr = symbolTable.createTemporarySymbol(type::pointer(storeType));
        field.addressName = addr.getName();
        auto zero = symbolTable.createTemporarySymbol(storeType);
        field.zeroInitialize = true;
        field.sourceName = zero.getName();
        plan.push_back(std::move(field));
    }

    void zeroRegion(int offsetBytes, const type::Type& t) {
        if (t.isUnion()) {
            emitZero(offsetBytes, t);
            return;
        }
        if (t.isStructure()) {
            for (int i = 0; i < t.memberCount(); ++i) {
                std::string name;
                type::Type mt = type::voidType();
                int off = 0;
                if (!t.memberAt(i, name, mt, off)) {
                    break;
                }
                zeroRegion(offsetBytes + off, mt);
            }
            return;
        }
        if (t.isArray()) {
            const int n = t.getArraySize();
            if (n <= 0) {
                error("array brace initializers for incomplete arrays are not implemented");
                return;
            }
            const int stride = t.getElementStride();
            const type::Type elem = t.getElementType();
            for (int i = 0; i < n; ++i) {
                zeroRegion(offsetBytes + i * stride, elem);
            }
            return;
        }
        emitZero(offsetBytes, t);
    }

    void onUnwritten(int offsetBytes, const type::Type& t) override {
        zeroRegion(offsetBytes, t);
    }

    void placeScalar(int offsetBytes, const type::Type& storeType, ast::Expression* value) override {
        symbols::StructFieldInit field;
        field.offsetBytes = offsetBytes;
        auto addr = symbolTable.createTemporarySymbol(type::pointer(storeType));
        field.addressName = addr.getName();
        if (value && value->hasResultSymbol(annotations)) {
            visitor.typeCheck(
                    assignSourceType(*value, storeType, annotations), storeType, context);
            field.zeroInitialize = false;
            field.sourceName = value->getResultSymbol(annotations)->getName();
        } else {
            auto zero = symbolTable.createTemporarySymbol(storeType);
            field.zeroInitialize = true;
            field.sourceName = zero.getName();
        }
        plan.push_back(std::move(field));
    }
};

// --- Global constant data-word sink ---

struct DataWordSink : AggregateInitSink {
    SemanticAnalysisVisitor& visitor;
    translation_unit::Context context;
    std::vector<std::string>& words;
    int wordCount;
    bool failed { false };

    DataWordSink(SemanticAnalysisVisitor& v, translation_unit::Context ctx,
            std::vector<std::string>& w, int wc)
            : visitor { v }, context { std::move(ctx) }, words { w }, wordCount { wc } {
    }

    bool ok() const override { return !failed; }

    void error(const std::string& message) override {
        failed = true;
        visitor.semanticError(message, context);
    }

    static std::string formatWord(unsigned long long v) {
        if (v > 0x7fffffffull) {
            std::ostringstream hex;
            hex << "0x" << std::hex << v;
            return hex.str();
        }
        return std::to_string(v);
    }

    static bool parseWord(const std::string& s, unsigned long long& out) {
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

    void storeAt(int offsetBytes, long value, int storeSizeBytes) {
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

    void onUnwritten(int /*offsetBytes*/, const type::Type& /*t*/) override {
        // Words prefilled with "0".
    }

    void placeScalar(int offsetBytes, const type::Type& storeType, ast::Expression* value) override {
        if (!value) {
            return;
        }
        long v = 0;
        if (!value->evaluateConstant(v)) {
            error("global brace initializer is not a constant expression");
            return;
        }
        storeAt(offsetBytes, v, storeType.getSize());
    }
};

} // namespace

void SemanticAnalysisVisitor::lowerLocalInitializer(ast::InitializedDeclarator& declarator,
        const type::Type& objectType) {
    if (!declarator.hasInitializer()) {
        return;
    }

    if (symbolTable.isAtFileScope()) {
        long initValue = 0;
        if (declarator.getInitializer()->evaluateConstant(initValue)) {
            symbolTable.setGlobalInitializer(declarator.getName(), initValue);
            return;
        }
        if (auto* list = dynamic_cast<ast::InitializerListExpression*>(declarator.getInitializer())) {
            if (!(objectType.isRecord() || objectType.isArray())) {
                if (list->getElements().size() == 1 && list->getElements().front().value
                        && list->getElements().front().value->evaluateConstant(initValue)) {
                    symbolTable.setGlobalInitializer(declarator.getName(), initValue);
                    return;
                }
                semanticError("global brace initializer is not a constant expression", declarator.getContext());
                return;
            }
            const int wordCount = type::object_abi::dataWords(objectType.getSize());
            if (wordCount <= 0) {
                return;
            }
            std::vector<std::string> words(static_cast<std::size_t>(wordCount), "0");
            DataWordSink sink { *this, declarator.getContext(), words, wordCount };
            walkAggregateInit(objectType, list, 0, sink);
            if (!sink.ok()) {
                return;
            }
            symbolTable.setGlobalMultiWordInitializer(declarator.getName(), std::move(words));
            return;
        }
        semanticError("global initializer is not a constant expression", declarator.getContext());
        return;
    }

    if (auto* list = dynamic_cast<ast::InitializerListExpression*>(declarator.getInitializer())) {
        if (objectType.isRecord() || objectType.isArray()) {
            std::vector<symbols::StructFieldInit> plan;
            FieldPlanSink sink { *this, symbolTable, annotations(), declarator.getContext(), plan };
            walkAggregateInit(objectType, list, 0, sink);
            if (sink.ok()) {
                annotations().setStructFieldInits(&declarator, std::move(plan));
            }
            return;
        }
        // Top-level scalar braces.
        if (list->getElements().size() > 1) {
            semanticError("excess elements in scalar initializer", declarator.getContext());
            return;
        }
        if (list->getElements().empty() || !list->getElements().front().value) {
            return;
        }
        ast::Expression* value = list->getElements().front().value.get();
        auto* nested = dynamic_cast<ast::InitializerListExpression*>(value);
        while (nested) {
            if (nested->getElements().size() > 1) {
                semanticError("excess elements in scalar initializer", declarator.getContext());
                return;
            }
            if (nested->getElements().empty() || !nested->getElements().front().value) {
                return;
            }
            value = nested->getElements().front().value.get();
            nested = dynamic_cast<ast::InitializerListExpression*>(value);
        }
        if (value && value->hasResultSymbol(annotations())) {
            type::Type src = assignSourceType(*value, objectType, annotations());
            if (!value->holdsAggregateAddress() || objectType.isPointer()) {
                typeCheck(src, objectType, declarator.getContext());
            }
            list->setResultSymbol(annotations(), *value->getResultSymbol(annotations()));
        }
        return;
    }

    ast::Expression* initExpr = declarator.getInitializer();
    if (initExpr && initExpr->hasResultSymbol(annotations())) {
        type::Type src = assignSourceType(*initExpr, objectType, annotations());
        if (!initExpr->holdsAggregateAddress() || objectType.isPointer()) {
            typeCheck(src, objectType, declarator.getContext());
        }
    }
}

} // namespace semantic_analyzer
