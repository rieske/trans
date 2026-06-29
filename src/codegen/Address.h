#ifndef ADDRESS_H_
#define ADDRESS_H_

#include <string>
#include <utility>

namespace codegen {

// Where a named object lives (not a register-allocation temp).
//
// Policy (interim, while Value still carries storage flags):
// - Locals: FrameBase::Rsp + slot offset; may also be register-cached on a Value.
// - Stack arguments: FrameBase::Rbp + arg offset; same cache model.
// - Globals: GlobalLabel; always committed through memory (bindResult stores).
// Phase 2+ will register Address in an object map and slim Value to temps/caches only.

enum class FrameBase {
    Rsp,
    Rbp
};

class Address {
public:
    static Address frame(FrameBase base, int offsetBytes, int sizeBytes) {
        return Address { Kind::Frame, base, offsetBytes, sizeBytes, {} };
    }

    static Address globalLabel(std::string label, int sizeBytes) {
        return Address { Kind::Global, FrameBase::Rsp, 0, sizeBytes, std::move(label) };
    }

    bool isGlobal() const { return kind_ == Kind::Global; }
    FrameBase frameBase() const { return frameBase_; }
    int offsetBytes() const { return offsetBytes_; }
    int sizeBytes() const { return sizeBytes_; }
    const std::string& label() const { return label_; }

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
