#ifndef SEMANTIC_CONSTANTADDRESS_H_
#define SEMANTIC_CONSTANTADDRESS_H_

#include "symbols/AnnotationStore.h"
#include "symbols/GlobalInitializer.h"
#include "types/Type.h"
#include "util/FloatingLiteral.h"

namespace ast {
class Expression;
}

namespace semantic_analyzer {

// Dest-directed static scalar fold (C 6.6). ICE and float literals convert to
// dest; bool-from-float is != 0.0. Address constants: string, &id / &id.m /
// &a[i], ptr+/-n, bare array and function designators. Address labels require
// a full machine-word store. Returns false if not a constant.
bool foldFloatingBits(const ast::Expression* expr, util::FloatingBits& out);

bool tryFoldGlobalInit(ast::Expression* expr, const type::Type& storeType,
        symbols::AnnotationStore& store, symbols::GlobalInitializer& out);

} // namespace semantic_analyzer

#endif // SEMANTIC_CONSTANTADDRESS_H_
