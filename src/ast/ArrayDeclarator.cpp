#include "ArrayDeclarator.h"

#include <limits>
#include <stdexcept>

#include "AbstractSyntaxTreeVisitor.h"

#include "Expression.h"
#include "types/Type.h"

namespace ast {

ArrayDeclarator::ArrayDeclarator(std::unique_ptr<DirectDeclarator> declarator, std::unique_ptr<Expression> subscriptExpression) :
		DirectDeclarator(declarator->getName(), declarator->getContext()),
		subscriptExpression { std::move(subscriptExpression) },
		baseDeclarator { std::move(declarator) } {
}

void ArrayDeclarator::accept(AbstractSyntaxTreeVisitor& visitor) {
	visitor.visit(*this);
}

void ArrayDeclarator::visitBaseDeclarator(AbstractSyntaxTreeVisitor& visitor) {
	baseDeclarator->accept(visitor);
}

DirectDeclarator& ArrayDeclarator::getBaseDeclarator() const {
	return *baseDeclarator;
}

void ArrayDeclarator::setArraySize(long size) {
	arraySize = size;
	arraySizeSet = true;
}

long ArrayDeclarator::getArraySize() const {
	return arraySize;
}

type::Type ArrayDeclarator::getFundamentalType(std::vector<Pointer> indirection, const type::Type& baseType) {
	// C: `int *a[N]` is array of N pointers (indirection applies to the element type).
	// Nested brackets compose from the inside out: T a[4][20] parses as
	// ArrayDeclarator(20, ArrayDeclarator(4, id)) and must become array[4] of array[20] of T
	// (outer dimension applied by the nested declarator).
	type::Type elementType = baseType;
	for (Pointer pointer : indirection) {
		elementType = type::pointer(elementType, pointer.getQualifiers());
	}
	// Incomplete array T name[] (and flexible members) - treat as zero-length array.
	if (!subscriptExpression) {
		return baseDeclarator->getFundamentalType({}, type::array(elementType, 0));
	}
	// Prefer size folded during semantic analysis (sizeof(arr[0]) needs a visit first).
	long length = 0;
	if (arraySizeSet && arraySize > 0) {
		length = arraySize;
	} else if (!subscriptExpression->evaluateConstant(length) || length < 0) {
		// Non-constant bounds (VLAs, e.g. parameter T a[n] in system headers) decay
		// to pointer-to-element, matching C parameter array adjustment.
		return baseDeclarator->getFundamentalType({}, type::pointer(elementType));
	}
	if (length > static_cast<long>(std::numeric_limits<int>::max())) {
		throw std::invalid_argument { "array size is too large" };
	}
	// GCC zero-length arrays appear in system headers as flexible array members.
	// type::array also rejects stride*count overflow (INT_MAX-sized int arrays, etc.).
	return baseDeclarator->getFundamentalType({}, type::array(elementType, static_cast<int>(length)));
}

} // namespace ast
