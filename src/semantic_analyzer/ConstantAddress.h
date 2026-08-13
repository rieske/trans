#ifndef SEMANTIC_CONSTANTADDRESS_H_
#define SEMANTIC_CONSTANTADDRESS_H_

#include <functional>
#include <string>

#include "symbols/AnnotationStore.h"
#include "symbols/GlobalInitializer.h"
#include "types/Type.h"

namespace ast {
class Expression;
class IdentifierExpression;
}

namespace semantic_analyzer {

// Default storage label: function designator, result symbol (mangled static), or bare id.
std::string defaultStorageLabel(ast::IdentifierExpression* id, symbols::AnnotationStore& store);

// Peel (T) casts, then resolve &id, &id.m, &a[i], ptr±n as an AddressInit.
// storageLabel maps IdentifierExpression bases to linker/storage names.
bool resolveAddressConstant(ast::Expression* expr, symbols::AddressInit& out,
        symbols::AnnotationStore& store,
        std::function<std::string(ast::IdentifierExpression*)> storageLabel);

// Peel outer TypeCast nodes; returns innermost expression (or null).
ast::Expression* peelTypeCasts(ast::Expression* expr);

// Dest-directed static scalar fold (C 6.6). ICE and float literals convert to
// dest; bool-from-float is != 0.0. Address constants: string, &id / &id.m /
// &a[i], ptr+/-n, bare array and function designators. Address labels require
// a full machine-word store. Returns false if not a constant.
bool tryFoldDataWord(ast::Expression* expr, const type::Type& storeType,
        symbols::AnnotationStore& store, symbols::DataWord& outWord);

} // namespace semantic_analyzer

#endif // SEMANTIC_CONSTANTADDRESS_H_
