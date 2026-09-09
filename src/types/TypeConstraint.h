#ifndef TYPES_TYPECONSTRAINT_H_
#define TYPES_TYPECONSTRAINT_H_

#include <limits>
#include <optional>

#include "Type.h"

namespace type {

// Incomplete as an array element or record member (void, function, incomplete record/array).
inline bool incompleteArrayElement(const Type& t) {
    return t.isVoid() || t.isFunction() || t.isIncompleteRecord() || t.isIncompleteArray();
}

// Byte size of a complete array layer. Empty on overflow. Incomplete element is 0.
inline std::optional<int> arrayByteSize(const Type& element, int count) {
    if (incompleteArrayElement(element) || count < 0) {
        return 0;
    }
    const long long stride = element.getSize();
    if (stride < 0) {
        return 0;
    }
    const long long bytes = stride * static_cast<long long>(count);
    if (bytes > static_cast<long long>(std::numeric_limits<int>::max())) {
        return std::nullopt;
    }
    return static_cast<int>(bytes);
}

inline const char* arrayLayerError(const Type& element, bool checkCount, int count) {
    if (incompleteArrayElement(element)) {
        return "array of incomplete type";
    }
    if (!checkCount || count < 0) {
        return nullptr;
    }
    if (!arrayByteSize(element, count)) {
        return "array size is too large";
    }
    return nullptr;
}

// C array constraint on a constructed Type. Walks arrays, pointers, and function
// return types. Function parameters are stored adjusted (void a[3] -> void*), so
// they are not visible here; use Declarator::arrayConstraintError for formals.
inline const char* arrayTypeError(const Type& t) {
    if (t.isArray()) {
        const bool sized = !t.isIncompleteArray() && !t.isVariableArray();
        if (const char* error = arrayLayerError(
                    t.getElementType(), sized, sized ? t.getArraySize() : 0)) {
            return error;
        }
        return arrayTypeError(t.getElementType());
    }
    if (t.isPointer()) {
        return arrayTypeError(t.dereference());
    }
    if (t.isFunction()) {
        return arrayTypeError(t.getFunction().getReturnType());
    }
    return nullptr;
}

} // namespace type

#endif // TYPES_TYPECONSTRAINT_H_
