#include "Value.h"

#include <cassert>

namespace codegen {

namespace {

type::sysv::Classification defaultClassification(ValueKind kind, int sizeInBytes, bool isSigned) {
    if (kind == ValueKind::COMPLEX) {
        return type::sysv::complexClass(sizeInBytes);
    }
    if (kind == ValueKind::FLOATING && sizeInBytes > 8) {
        return type::sysv::x87Scalar();
    }
    if (kind == ValueKind::FLOATING) {
        return type::sysv::sseScalar(sizeInBytes);
    }
    type::sysv::Classification c = type::sysv::integerScalar(sizeInBytes);
    if (sizeInBytes == 1 || sizeInBytes == 2 || sizeInBytes == 4) {
        c.gprExtend = isSigned ? type::sysv::GprExtend::Sign : type::sysv::GprExtend::Zero;
    }
    return c;
}

} // namespace

Value::Value(std::string name, int index, ValueKind kind, int sizeInBytes, bool isSigned) :
        Value { std::move(name), index, kind, sizeInBytes, isSigned,
                defaultClassification(kind, sizeInBytes, isSigned) }
{
}

Value::Value(std::string name, int index, ValueKind kind, int sizeInBytes, bool isSigned,
        type::sysv::Classification classification) :
        name { std::move(name) },
        index { index },
        valueKind_ { kind },
        sizeInBytes { sizeInBytes },
        signedIntegral { isSigned },
        classification { classification }
{
}

std::string Value::getName() const {
    return name;
}

void Value::assignRegister(Register* reg) {
    assignedRegister = reg;
}

bool Value::isStored() const {
    return !assignedRegister;
}

void Value::removeRegister(Register* reg) {
    if (reg == assignedRegister) {
        assignedRegister = nullptr;
    }
}

Register& Value::getAssignedRegister() const {
    assert(assignedRegister != nullptr);
    return *assignedRegister;
}

int Value::getIndex() const {
    return index;
}

ValueKind Value::getValueKind() const {
    return valueKind_;
}

int Value::getSizeInBytes() const {
    return sizeInBytes;
}

bool Value::isSigned() const {
    return signedIntegral;
}

type::sysv::Classification Value::getClassification() const {
    return classification;
}

void Value::setLastUseOrdinal(int ordinal) {
    lastUseOrdinal = ordinal;
}

int Value::getLastUseOrdinal() const {
    return lastUseOrdinal;
}

} // namespace codegen
