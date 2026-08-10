#include "SysVClassify.h"

#include "ObjectAbi.h"
#include "Primitive.h"

namespace type {
namespace sysv {

namespace {

constexpr int MAX_CLASSIFY_BYTES = 8 * type::object_abi::MACHINE_WORD_SIZE;

Class merge(Class a, Class b) {
    if (a == b) {
        return a;
    }
    if (a == Class::NoClass) {
        return b;
    }
    if (b == Class::NoClass) {
        return a;
    }
    if (a == Class::Memory || b == Class::Memory) {
        return Class::Memory;
    }
    if (a == Class::Integer || b == Class::Integer) {
        return Class::Integer;
    }
    if (isX87(a) || isX87(b) || a == Class::SseUp || b == Class::SseUp) {
        return Class::Memory;
    }
    return Class::Sse;
}

void postMerge(int sizeBytes, Class& lo, Class& hi) {
    if (hi == Class::Memory) {
        lo = Class::Memory;
    }
    if (hi == Class::X87Up && lo != Class::X87) {
        lo = Class::Memory;
    }
    if (sizeBytes > type::object_abi::REGISTER_RETURN_MAX_BYTES
            && !(lo == Class::Sse && hi == Class::SseUp)) {
        lo = Class::Memory;
    }
    if (hi == Class::SseUp && lo != Class::Sse) {
        hi = Class::Sse;
    }
    if (lo == Class::Memory) {
        hi = Class::Memory;
    }
}

void classifyInto(const Type& t, int offsetBase, Class& lo, Class& hi) {
    lo = Class::NoClass;
    hi = Class::NoClass;
    Class& current = offsetBase < 8 ? lo : hi;

    if (t.isVoid()) {
        return;
    }
    if (t.isPointer() || t.isFunction()) {
        current = Class::Integer;
        return;
    }
    if (t.isPrimitive()) {
        const Primitive p = t.getPrimitive();
        if (p.kind() == PrimitiveKind::LongDouble) {
            lo = Class::X87;
            hi = Class::X87Up;
            return;
        }
        if (p.isFloating()) {
            current = Class::Sse;
            return;
        }
        current = Class::Integer;
        return;
    }
    if (t.isArray()) {
        if (t.isIncompleteArray()) {
            return;
        }
        const Type elem = t.getElementType();
        const int stride = t.getElementStride();
        const int n = t.getArraySize();
        if (t.getSize() > MAX_CLASSIFY_BYTES) {
            lo = Class::Memory;
            return;
        }
        for (int i = 0; i < n; ++i) {
            Class flo = Class::NoClass;
            Class fhi = Class::NoClass;
            classifyInto(elem, offsetBase + i * stride, flo, fhi);
            lo = merge(lo, flo);
            hi = merge(hi, fhi);
            if (lo == Class::Memory) {
                return;
            }
        }
        return;
    }
    if (t.isRecord()) {
        if (!t.isCompleteRecord()) {
            return;
        }
        if (t.getSize() > MAX_CLASSIFY_BYTES) {
            lo = Class::Memory;
            return;
        }
        for (const auto& member : t.getMembers()) {
            if (!member.type) {
                continue;
            }
            const int align = member.type->getAlignment();
            if (align > 1 && (member.offsetBytes % align) != 0) {
                lo = Class::Memory;
                return;
            }
            Class flo = Class::NoClass;
            Class fhi = Class::NoClass;
            classifyInto(*member.type, offsetBase + member.offsetBytes, flo, fhi);
            lo = merge(lo, flo);
            hi = merge(hi, fhi);
            if (lo == Class::Memory) {
                return;
            }
        }
        return;
    }
}

} // namespace

Classification classify(const Type& t) {
    Classification result;
    Class lo = Class::NoClass;
    Class hi = Class::NoClass;
    classifyInto(t, 0, lo, hi);
    postMerge(t.getSize(), lo, hi);
    if (lo == Class::Memory) {
        result.memory = true;
        return result;
    }
    if (lo == Class::NoClass) {
        return result;
    }
    result.eightbytes[0] = lo;
    result.count = 1;
    if (hi != Class::NoClass) {
        result.eightbytes[1] = hi;
        result.count = 2;
    }
    return result;
}

} // namespace sysv
} // namespace type
