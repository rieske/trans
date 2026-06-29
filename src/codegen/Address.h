#ifndef ADDRESS_H_
#define ADDRESS_H_

#include <cassert>
#include <string>
#include <utility>

namespace codegen {

// Where a named object lives (not a register-allocation temp).
//
// Policy:
// - Locals / register-arg spill slots: StackPointer + offset; Value may cache in a register.
// - Stack arguments: BasePointer + offset; same cache model for the Value.
// - Globals: label in globalHomes; stores via bindResult; Values are resolve shells only
//   (loads use scratch registers - never assign the global Value to a register).
// - Homes are only in globalHomes / frameHomes.

// Which pointer register a frame slot is addressed from (not an ISA-specific mnemonic).
enum class FrameBase {
    StackPointer,
    BasePointer
};

class Address {
public:
    static Address frame(FrameBase base, int offsetBytes, int sizeBytes) {
        return Address { Kind::Frame, base, offsetBytes, sizeBytes, {} };
    }

    static Address globalLabel(std::string label, int sizeBytes) {
        return Address { Kind::Global, FrameBase::StackPointer, 0, sizeBytes, std::move(label) };
    }

    bool isGlobal() const { return kind_ == Kind::Global; }

    FrameBase frameBase() const {
        assert(!isGlobal() && "frameBase() is only valid for frame slots");
        return frameBase_;
    }

    int offsetBytes() const {
        assert(!isGlobal() && "offsetBytes() is only valid for frame slots");
        return offsetBytes_;
    }

    int sizeBytes() const { return sizeBytes_; }

    const std::string& label() const {
        assert(isGlobal() && "label() is only valid for global homes");
        return label_;
    }

private:
    enum class Kind { Frame, Global };

    Address(Kind kind, FrameBase frameBase, int offsetBytes, int sizeBytes, std::string label) :
            kind_ { kind },
            frameBase_ { frameBase },
            offsetBytes_ { offsetBytes },
            sizeBytes_ { sizeBytes },
            label_ { std::move(label) } {}

    Kind kind_;
    FrameBase frameBase_;
    int offsetBytes_;
    int sizeBytes_;
    std::string label_;
};

} // namespace codegen

#endif // ADDRESS_H_

