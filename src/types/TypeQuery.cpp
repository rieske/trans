#include "TypeQuery.h"
#include "util/StringLiteralDecode.h"

namespace type {

bool productCanAssignFrom(const Type& dest, const Type& source) {
    // Destination dest, source source. Product rules are looser than ISO C
    // (git-shaped C): reject clear mismatches (struct/scalar, different struct
    // bodies, void). Not ISO can_assign. Sole implementation of product assign
    // (lives with TypeQuery policy helpers, not structural Type layout).
    //
    // Arrays decay on both sides: member-array expressions keep expressionType
    // as T[N] for sizeof while their result symbol is already a pointer.
    Type dst = dest;
    Type src = source;
    if (dst.isArray()) {
        dst = dst.decayArray();
    }
    if (src.isArray()) {
        src = src.decayArray();
    }
    if (src.isFunction()) {
        src = pointer(src);
    }
    if (dst.isFunction()) {
        return false;
    }

    if (dst.isVoid() || src.isVoid()) {
        return false;
    }

    if (dst.isRecord() || src.isRecord()) {
        if (dst.isRecord() && src.isRecord()) {
            return dst.structureBodyIdentity() == src.structureBodyIdentity();
        }
        if (dst.isPointer() || src.isPointer()) {
            return true;
        }
        return false;
    }

    return isProductScalar(dst) && isProductScalar(src);
}

int sizeofStringLiteralTokenBytes(const std::string& token) {
    return util::stringLiteralArrayLength(token);
}

} // namespace type
