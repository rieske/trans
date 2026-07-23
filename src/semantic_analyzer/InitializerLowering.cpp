#include "InitializerLowering.h"

#include <functional>
#include <optional>
#include <map>
#include <sstream>

#include "ConstantAddress.h"
#include "SymbolTable.h"
#include "ast/ArrayAccess.h"
#include "ast/DoubleOperandExpression.h"
#include "ast/IdentifierExpression.h"
#include "ast/InitializerListExpression.h"
#include "ast/MemberAccess.h"
#include "ast/StringLiteralExpression.h"
#include "ast/TypeCast.h"
#include "ast/UnaryExpression.h"
#include "types/ObjectAbi.h"
#include "util/StringLiteralDecode.h"

namespace semantic_analyzer {

namespace {

int incompleteArrayLengthFromList(const ast::InitializerListExpression* list) {
    int length = 0;
    int positional = 0;
    for (const auto& element : list->getElements()) {
        if (!element.value) {
            continue;
        }
        if (element.arrayIndex) {
            int idx = static_cast<int>(*element.arrayIndex);
            if (idx >= 0 && idx + 1 > length) {
                length = idx + 1;
            }
            positional = idx + 1;
        } else {
            if (positional + 1 > length) {
                length = positional + 1;
            }
            ++positional;
        }
    }
    return length;
}

} // namespace

type::Type completeArrayTypeFromList(const type::Type& arrayType,
        const ast::InitializerListExpression* list) {
    if (!arrayType.isArray() || arrayType.getArraySize() != 0 || !list) {
        return arrayType;
    }
    int length = incompleteArrayLengthFromList(list);
    if (length < 1) {
        length = 1;
    }
    return type::array(arrayType.getElementType(), length);
}

type::Type completeArrayTypeFromString(const type::Type& arrayType,
        const ast::StringLiteralExpression* strLit) {
    if (!arrayType.isArray() || arrayType.getArraySize() != 0 || !strLit) {
        return arrayType;
    }
    const int length = util::stringLiteralArrayLength(strLit->getValue());
    return type::array(arrayType.getElementType(), length);
}

} // namespace semantic_analyzer
