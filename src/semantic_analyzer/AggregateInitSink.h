#ifndef AGGREGATE_INIT_SINK_H_
#define AGGREGATE_INIT_SINK_H_

#include "types/ObjectAbi.h"
#include "types/Type.h"

#include <string>

namespace ast {
class Expression;
}

namespace semantic_analyzer {

// Policy sink for one placement pass over an aggregate initializer.
struct AggregateInitSink {
    virtual ~AggregateInitSink() = default;
    virtual void placeScalar(const type::FoundMember& slot, ast::Expression* value) = 0;
    virtual void placeInteger(const type::FoundMember& slot, long value) = 0;
    virtual void onUnwritten(const type::FoundMember& slot) = 0;
    virtual void error(const std::string& message) = 0;
    virtual bool ok() const = 0;
};

// Shared layout walk: invoke leaf(offset, storeType) for each scalar storage unit.
// Unions are packed as machine-word / int / byte units (codegen store widths).
// incompleteArrayError is called if an incomplete array is encountered; return true to abort.
template<typename Leaf, typename Incomplete>
void forEachInitStorageUnit(const type::Type& t, int offsetBytes, Leaf&& leaf,
        Incomplete&& incompleteArrayError) {
    if (t.isUnion()) {
        const int size = t.getSize();
        int off = 0;
        while (off + type::object_abi::MACHINE_WORD_SIZE <= size) {
            leaf(offsetBytes + off, type::signedLong());
            off += type::object_abi::MACHINE_WORD_SIZE;
        }
        if (off + 4 <= size) {
            leaf(offsetBytes + off, type::signedInteger());
            off += 4;
        }
        while (off < size) {
            leaf(offsetBytes + off, type::signedCharacter());
            ++off;
        }
        return;
    }
    if (t.isStructure()) {
        for (const auto& member : t.getMembers()) {
            if (!member.type) {
                continue;
            }
            if (member.isBitField()) {
                leaf(offsetBytes + member.offsetBytes, *member.type);
                continue;
            }
            forEachInitStorageUnit(*member.type, offsetBytes + member.offsetBytes, leaf,
                    incompleteArrayError);
        }
        return;
    }
    if (t.isArray()) {
        const int n = t.getArraySize();
        if (n <= 0) {
            incompleteArrayError();
            return;
        }
        const int stride = t.getElementStride();
        const type::Type elem = t.getElementType();
        for (int i = 0; i < n; ++i) {
            forEachInitStorageUnit(elem, offsetBytes + i * stride, leaf, incompleteArrayError);
        }
        return;
    }
    leaf(offsetBytes, t);
}

} // namespace semantic_analyzer

#endif
