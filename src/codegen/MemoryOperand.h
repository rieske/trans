#ifndef MEMORYOPERAND_H_
#define MEMORYOPERAND_H_

#include <string>
#include <utility>

namespace codegen {

class Register;

// A memory location operand: either base-register-plus-offset (`[base + offset]`,
// used for stack slots and pointer dereferences) or a global variable (a named label).
// The instruction set renders it in its own syntax, so the global/stack distinction
// lives in one place instead of at every call site.
class MemoryOperand {
public:
    static MemoryOperand at(const Register& base, int offset) {
        return MemoryOperand { &base, offset, {} };
    }
    static MemoryOperand global(std::string label) {
        return MemoryOperand { nullptr, 0, std::move(label) };
    }

    bool isGlobal() const { return base_ == nullptr; }
    const Register& baseRegister() const { return *base_; }
    int offset() const { return offset_; }
    const std::string& label() const { return label_; }

private:
    MemoryOperand(const Register* base, int offset, std::string label) :
            base_ { base }, offset_ { offset }, label_ { std::move(label) } {}

    const Register* base_;
    int offset_;
    std::string label_;
};

} // namespace codegen

#endif // MEMORYOPERAND_H_
