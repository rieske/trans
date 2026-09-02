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

// Cover `size` bytes as codegen store widths (word, then 4, then bytes).
template<typename Leaf>
void forEachRepresentationUnit(int size, int offsetBytes, Leaf&& leaf) {
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
}

// Implicit zero of an unwritten aggregate: the object representation, not members.
template<typename Leaf>
void forEachUnwrittenRepresentation(const type::Type& t, int offsetBytes, Leaf&& leaf) {
    if (t.isArray() && t.getArraySize() <= 0) {
        return;
    }
    forEachRepresentationUnit(t.getSize(), offsetBytes, leaf);
}

} // namespace semantic_analyzer

#endif
