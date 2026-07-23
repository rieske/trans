#ifndef SEMANTIC_CONSTANTADDRESS_H_
#define SEMANTIC_CONSTANTADDRESS_H_

#include <functional>
#include <string>

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
std::string defaultStorageLabel(ast::IdentifierExpression* id);

// Peel (T) casts, then resolve &id, &id.m, &a[i], ptr±n as an address constant.
// storageLabel maps IdentifierExpression bases to linker/storage names.
bool resolveAddressConstant(ast::Expression* expr, AddressConstant& out,
        std::function<std::string(ast::IdentifierExpression*)> storageLabel = defaultStorageLabel);

// Peel outer TypeCast nodes; returns innermost expression (or null).
ast::Expression* peelTypeCasts(ast::Expression* expr);

} // namespace semantic_analyzer

#endif // SEMANTIC_CONSTANTADDRESS_H_
