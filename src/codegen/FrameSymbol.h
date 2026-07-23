#ifndef CODEGEN_FRAMESYMBOL_H_
#define CODEGEN_FRAMESYMBOL_H_

#include <string>

#include "Value.h"
#include "symbols/ValueEntry.h"
#include "types/Type.h"
#include "types/TypeQuery.h"

namespace codegen {

// Bridge from semantic ValueEntry to stack/register Value: keeps full type::Type
// so size/signedness/floating policy is derived once via TypeQuery (not hand-rolled).
struct FrameSymbol {
    std::string name;
    int index { 0 };
    type::Type type { type::voidType() };
    bool isTemp { false };

    int sizeBytes() const {
        return type::valueSizeBytes(type);
    }

    bool isSigned() const {
        return type::valueIsSigned(type);
    }

    bool isFloating() const {
        return type::isFloating(type);
    }

    ValueKind valueKind() const {
        return isFloating() ? ValueKind::FLOATING : ValueKind::INTEGRAL;
    }

    // Uses ValueEntry index (formals / pre-assigned homes).
    Value toValue() const {
        return toValueAtSlot(index);
    }

    // Frame packing assigns stack slots independently of ValueEntry index.
    Value toValueAtSlot(int stackSlot) const {
        return Value { name, stackSlot, valueKind(), sizeBytes(), isSigned() };
    }
};

inline FrameSymbol frameSymbolFrom(const symbols::ValueEntry& entry, bool isTemp = false) {
    FrameSymbol fs;
    fs.name = entry.getName();
    fs.index = entry.getIndex();
    fs.type = entry.getType();
    fs.isTemp = isTemp;
    return fs;
}

} // namespace codegen

#endif // CODEGEN_FRAMESYMBOL_H_
