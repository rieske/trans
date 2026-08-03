#ifndef SEMANTIC_CONSTANTADDRESS_H_
#define SEMANTIC_CONSTANTADDRESS_H_

#include <functional>
#include <string>

#include "symbols/AnnotationStore.h"
#include "types/Type.h"

namespace ast {
class Expression;
class IdentifierExpression;
}

namespace semantic_analyzer {

// Resolved C address constant: label + optional byte offset (label+N form).
struct AddressConstant {
    std::string label;
    long byteOffset { 0 };

    // Encoding used in .data address initializers: "label" or "label+N".
    std::string toOperand() const;
};

// Default storage label: function designator, result symbol (mangled static), or bare id.
std::string defaultStorageLabel(ast::IdentifierExpression* id, symbols::AnnotationStore& store);

// Peel (T) casts, then resolve &id, &id.m, &a[i], ptr±n as an address constant.
// storageLabel maps IdentifierExpression bases to linker/storage names.
bool resolveAddressConstant(ast::Expression* expr, AddressConstant& out,
        symbols::AnnotationStore& store,
        std::function<std::string(ast::IdentifierExpression*)> storageLabel);

// Peel outer TypeCast nodes; returns innermost expression (or null).
ast::Expression* peelTypeCasts(ast::Expression* expr);

// Format an integer for .data multi-word packing (decimal, or hex when large).
std::string formatDataWord(unsigned long long v);

// Fold a file-scope initializer expression to a .data operand string.
// Handles peeled casts, integer constants, string addresses, &id / &id.m,
// bare array and function designators. Labels require a full machine-word store.
// storeType gates sub-word address packs. Returns false if not a constant.
bool tryFoldDataOperand(ast::Expression* expr, const type::Type& storeType,
        symbols::AnnotationStore& store, std::string& outOperand);

} // namespace semantic_analyzer

#endif // SEMANTIC_CONSTANTADDRESS_H_
