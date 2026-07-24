#include "InitializerLowering.h"
#include "InitializerLoweringInternal.h"

#include <optional>

#include "SymbolTable.h"
#include "ast/InitializerListExpression.h"
#include "ast/StringLiteralExpression.h"
#include "util/StringLiteralDecode.h"
#include "Symbols.h"

namespace semantic_analyzer {


namespace {

struct FieldInitSinkAdapter : init_detail::AggregateInitSink {
    SymbolTable& symbolTable;
    FieldInitSink sink;

    FieldInitSinkAdapter(SymbolTable& st, FieldInitSink s)
            : symbolTable { st }, sink { std::move(s) } {
    }

    void addStore(int offsetBytes, const type::Type& storeType,
            ValueEntry* source, bool zero,
            std::optional<std::string> addressOfOperand = std::nullopt,
            std::optional<std::string> constantValue = std::nullopt) {
        ast::StructFieldInit init;
        init.offsetBytes = offsetBytes;
        init.zeroInitialize = zero;
        init.constantValue = constantValue;
        init.addressOfOperand = addressOfOperand;
        if (addressOfOperand) {
            init.source = std::make_unique<ValueEntry>(
                    symbolTable.createTemporarySymbol(storeType));
        } else if (zero) {
            init.source = std::make_unique<ValueEntry>(
                    symbolTable.createTemporarySymbol(storeType));
        } else if (source) {
            init.source = std::make_unique<ValueEntry>(*source);
        } else if (constantValue) {
            init.source = std::make_unique<ValueEntry>(
                    symbolTable.createTemporarySymbol(storeType));
        } else {
            return;
        }
        init.address = std::make_unique<ValueEntry>(
                symbolTable.createTemporarySymbol(type::pointer(storeType)));
        sink(std::move(init));
    }

    void zeroRegion(int offsetBytes, const type::Type& t) {
        if (t.isUnion()) {
            const int size = t.getSize();
            int off = 0;
            while (off + 8 <= size) {
                addStore(offsetBytes + off, type::unsignedLong(), nullptr, true);
                off += 8;
            }
            while (off < size) {
                addStore(offsetBytes + off, type::unsignedCharacter(), nullptr, true);
                ++off;
            }
            return;
        }
        if (t.isRecord()) {
            for (const auto& nested : t.getStructMembers()) {
                if (nested.type) {
                    zeroRegion(offsetBytes + nested.offsetBytes, *nested.type);
                }
            }
            return;
        }
        if (t.isArray()) {
            const int length = t.getArraySize();
            if (length <= 0) {
                return;
            }
            const int stride = t.getElementStride();
            const type::Type elementType = t.getElementType();
            for (int i = 0; i < length; ++i) {
                zeroRegion(offsetBytes + i * stride, elementType);
            }
            return;
        }
        addStore(offsetBytes, t, nullptr, true);
    }

    void placeScalar(int offsetBytes, const type::Type& storeType,
            ast::Expression* value) override {
        ValueEntry* source =
                (value && value->hasResultSymbol()) ? value->getResultSymbol() : nullptr;
        if (source) {
            if (storeType.isPointer() && source->getType().isArray()) {
                addStore(offsetBytes, storeType, source, false, source->getName());
            } else {
                addStore(offsetBytes, storeType, source, false);
            }
            return;
        }
        long v = 0;
        if (value && value->evaluateConstant(v)) {
            addStore(offsetBytes, storeType, nullptr, false, std::nullopt, std::to_string(v));
            return;
        }
        zeroRegion(offsetBytes, storeType);
    }

    bool placeStringArray(int offsetBytes, const type::Type& arrayType,
            ast::Expression* value) override {
        auto* strLit = dynamic_cast<ast::StringLiteralExpression*>(value);
        if (!strLit || !init_detail::isCharArrayType(arrayType)) {
            return false;
        }
        const type::Type elemType = arrayType.getElementType();
        const auto bytes = util::decodeStringLiteralBytes(strLit->getValue());
        const int arrLen = arrayType.getArraySize();
        const int stride = arrayType.getElementStride();
        const int n = arrLen > 0 ? arrLen : static_cast<int>(bytes.size());
        for (int i = 0; i < n; ++i) {
            unsigned char byte = (i < static_cast<int>(bytes.size()))
                    ? bytes[static_cast<std::size_t>(i)] : 0;
            addStore(offsetBytes + i * stride, elemType, nullptr, false, std::nullopt,
                    std::to_string(static_cast<unsigned>(byte)));
        }
        return true;
    }

    void placeAggregateCopy(int offsetBytes, const type::Type& storeType,
            ValueEntry* source) override {
        addStore(offsetBytes, storeType, source, false);
    }

    void onUnwritten(int offsetBytes, const type::Type& t) override {
        zeroRegion(offsetBytes, t);
    }
};

} // namespace

type::Type lowerToFieldInits(type::Type objectType,
        ast::Expression* initializer,
        SymbolTable& symbolTable,
        FieldInitSink sink) {
    if (!initializer) {
        return objectType;
    }

    FieldInitSinkAdapter adapter { symbolTable, std::move(sink) };

    if (auto* strLit = dynamic_cast<ast::StringLiteralExpression*>(initializer)) {
        if (objectType.isArray()) {
            if (objectType.getArraySize() == 0) {
                objectType = completeArrayTypeFromString(objectType, strLit);
            }
            adapter.placeStringArray(0, objectType, initializer);
        }
        return objectType;
    }

    auto* list = dynamic_cast<ast::InitializerListExpression*>(initializer);
    if (!list) {
        return objectType;
    }

    if (objectType.isArray() && objectType.getArraySize() == 0) {
        objectType = completeArrayTypeFromList(objectType, list);
    }

    if (objectType.isAggregate()) {
        init_detail::walkAggregateInit(objectType, list, 0, adapter);
    }
    return objectType;
}

} // namespace semantic_analyzer
