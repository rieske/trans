#include "DeclarationAnalyzer.h"

#include "ConstantAddress.h"
#include "InitializerLowering.h"
#include "ArrayDecay.h"

#include <cstring>
#include <vector>

#include "ast/ConstantExpression.h"
#include "ast/IdentifierExpression.h"
#include "ast/InitializedDeclarator.h"
#include "ast/InitializerListExpression.h"
#include "ast/StringLiteralExpression.h"
#include "util/StringLiteralDecode.h"
#include "types/TypeQuery.h"

namespace semantic_analyzer {

// Encode a double as IEEE-754 bit pattern stored in a long (for .data init).
static long doubleBitsAsLong(double d) {
    unsigned long long bits = 0;
    static_assert(sizeof(double) == sizeof(unsigned long long), "double must be 64-bit");
    std::memcpy(&bits, &d, sizeof(bits));
    return static_cast<long>(bits);
}

// C allows refining an empty variadic prototype (int f(...)) with a more specific
// fixed-arg list (int f(const char*, ...)), and identical redeclarations.
// Top-level const/volatile on parameters is ignored for compatibility (C 6.7.6.3).
bool functionTypesCompatible(const type::Function& existing, const type::Function& incoming) {
    // Structural type equality (ignores top-level const/volatile on params/return).
    if (existing.getReturnType().equivalentTo(incoming.getReturnType())) {
        const auto existingArgs = existing.getArguments();
        const auto newArgs = incoming.getArguments();
        if (existingArgs.size() == newArgs.size()) {
            bool argsMatch = true;
            for (std::size_t i = 0; i < existingArgs.size(); ++i) {
                if (!existingArgs[i].equivalentTo(newArgs[i])) {
                    argsMatch = false;
                    break;
                }
            }
            if (argsMatch && existing.isVariadic() == incoming.isVariadic()) {
                return true;
            }
        }
    }
    const auto existingArgs = existing.getArguments();
    const auto newArgs = incoming.getArguments();
    // Builtin empty-vararg (0 fixed args) may be refined by a header prototype with
    // fixed parameters + "...", and vice versa for identical linkage.
    if (existing.isVariadic() && incoming.isVariadic()
            && (existingArgs.empty() || newArgs.empty())
            && existing.getReturnType().equivalentTo(incoming.getReturnType())) {
        return true;
    }
    return false;
}

DeclarationAnalyzer::DeclarationAnalyzer(SymbolTable& symbols,
        SemanticDiagnostics diagnostics,
        ast::AbstractSyntaxTreeVisitor& childVisitor) :
        symbolTable { symbols },
        diagnostics { std::move(diagnostics) },
        childVisitor { childVisitor }
{
}

void DeclarationAnalyzer::analyze(ast::Declaration& declaration) {
    auto declSpecs = declaration.getDeclarationSpecifiers();
    declSpecs.accept(childVisitor);

    // typedef only introduces a type alias; no runtime symbol.
    bool isTypedef = false;
    for (const auto& storage : declSpecs.getStorageSpecifiers()) {
        if (storage.getStorage() == ast::Storage::TYPEDEF) {
            isTypedef = true;
            break;
        }
    }
    if (isTypedef) {
        // Still visit declarators so nested constructs are analyzed if needed.
        for (const auto& declarator : declaration.getDeclarators()) {
            declarator->accept(childVisitor);
        }
        return;
    }

    auto baseType = declSpecs.getResolvedType();
    bool isExtern = false;
    bool isStaticStorage = false;
    for (const auto& storage : declSpecs.getStorageSpecifiers()) {
        if (storage.getStorage() == ast::Storage::EXTERN) {
            isExtern = true;
        } else if (storage.getStorage() == ast::Storage::STATIC) {
            isStaticStorage = true;
        }
    }
    for (const auto& declarator : declaration.getDeclarators()) {
        declarator->accept(childVisitor);
    }

    for (const auto& declarator : declaration.getDeclarators()) {
        analyzeInitializedDeclarator(*declarator, baseType, isExtern, isStaticStorage);
    }
}

void DeclarationAnalyzer::analyzeInitializedDeclarator(ast::InitializedDeclarator& declarator,
        const type::Type& baseType,
        bool isExtern,
        bool isStaticStorage) {
    auto type = declarator.getFundamentalType(baseType);
    if (type.isFunction()) {
        // Function prototype.
        if (symbolTable.hasGlobalVariable(declarator.getName())) {
            diagnostics.error("function `" + declarator.getName() + "` conflicts with global variable of the same name",
                    declarator.getContext());
            return;
        }
        if (symbolTable.hasFunction(declarator.getName())) {
            auto existing = symbolTable.findFunction(declarator.getName());
            if (!functionTypesCompatible(existing.getType(), type.getFunction())) {
                diagnostics.error("function `" + declarator.getName() + "` declaration conflicts with previous one on "
                        + to_string(existing.getContext()), declarator.getContext());
            }
        } else {
            symbolTable.insertFunction(declarator.getName(), type.getFunction(), declarator.getContext());
        }
        return;
    }
    if (type.isVoid()) {
        diagnostics.error("variable `" + declarator.getName() + "` declared void", declarator.getContext());
    } else if (symbolTable.isAtFileScope() && symbolTable.hasFunction(declarator.getName())) {
        diagnostics.error("symbol `" + declarator.getName() + "` declaration conflicts with function of the same name",
                declarator.getContext());
    } else if (isExtern && !symbolTable.isAtFileScope() && !declarator.hasInitializer()) {
        // Block-scope extern: external linkage, no automatic storage.
        if (!symbolTable.insertExternLocal(declarator.getName(), type, declarator.getContext())) {
            diagnostics.error("symbol `" + declarator.getName() + "` declaration conflicts with previous one",
                    declarator.getContext());
        } else {
            declarator.setHolder(symbolTable.lookup(declarator.getName()));
        }
    } else if (isStaticStorage && !symbolTable.isAtFileScope()
            ? symbolTable.insertStaticLocal(declarator.getName(), type, declarator.getContext())
            : symbolTable.insertSymbol(declarator.getName(), type, declarator.getContext())) {
        // C 6.2.1: scope begins after the declarator completes, so the name is in
        // scope for the initializer (sizeof(*p), value self-ref, &p, etc.).
        declarator.setHolder(symbolTable.lookup(declarator.getName()));
        if (symbolTable.isAtFileScope()) {
            // A non-extern declaration (or any with an initializer) is a definition:
            // clear any prior pure-extern mark from a header prototype.
            if (isExtern && !declarator.hasInitializer()) {
                symbolTable.setGlobalExternal(declarator.getName(), true);
            } else {
                symbolTable.setGlobalExternal(declarator.getName(), false);
            }
            if (isStaticStorage) {
                symbolTable.setGlobalStaticStorage(declarator.getName(), true);
            }
        }
        if (declarator.hasInitializer()) {
            declarator.getInitializer()->accept(childVisitor);
            // T *p = arr; decay array initializer in place (same model as call/assign).
            if (type.isPointer()) {
                decayArrayInPlace(declarator.getInitializer(), symbolTable);
            }
            // Skip brace/string aggregate init (lowered separately); check expression
            // initializers against the (possibly decayed) holder type.
            auto* initExpr = declarator.getInitializer();
            const bool braceOrString =
                    dynamic_cast<ast::InitializerListExpression*>(initExpr) != nullptr
                    || dynamic_cast<ast::StringLiteralExpression*>(initExpr) != nullptr;
            if (!braceOrString && initExpr->hasResultSymbol()) {
                const type::Type& from = initExpr->getResultSymbol()->getType();
                // Holder type may still be incomplete array; use analyzed `type`.
                if (!type.canAssignFrom(from)) {
                    diagnostics.error(
                            "type mismatch: can't convert " + from.to_string() + " to "
                                    + type.to_string(),
                            declarator.getContext());
                }
            }
        }
        // File-scope and function-scope static: constant init goes into .data
        // (holder isGlobal for both; static locals use mangled holder names).
        if (declarator.hasInitializer() && declarator.getHolder()->isGlobal()) {
            const std::string storageName = declarator.getHolder()->getName();
            long initValue = 0;
            if (auto* strLit = dynamic_cast<ast::StringLiteralExpression*>(declarator.getInitializer())) {
                if (type.isArray()) {
                    // const char name[] = "..." (and char name[] = "..."): embed bytes.
                    // Complete incomplete arrays so sizeof / ARRAY_SIZE see the true length
                    // including NUL (git wrapper.c: static const char x_pattern[] = "XXXXXX").
                    if (type.getArraySize() == 0) {
                        const int length = util::stringLiteralArrayLength(strLit->getValue());
                        type = type::array(type.getElementType(), length);
                        // storageName is mangled for function-scope statics; also update
                        // the function-scope entry so lookup/sizeof see the completed type.
                        symbolTable.setGlobalType(storageName, type);
                        symbolTable.setLocalType(declarator.getName(), type);
                        declarator.setHolder(symbolTable.lookup(declarator.getName()));
                    }
                    symbolTable.setGlobalStringInitializer(storageName, strLit->getValue());
                } else if (type.isPointer()) {
                    // const char *name = "...": store address of the string constant
                    // (git: const char *blob_type = "blob"). Initializer was already
                    // visited above, so getConstantSymbol() is set.
                    symbolTable.setGlobalAddressInitializer(
                            storageName, strLit->getConstantSymbol());
                } else {
                    diagnostics.error("string literal initializer requires array or pointer type",
                            declarator.getContext());
                }
            } else if (type.isPrimitive() && type.getPrimitive().isFloating()) {
                // Store IEEE bits so the data section holds a real double/float.
                if (auto* fconst = dynamic_cast<ast::ConstantExpression*>(declarator.getInitializer())) {
                    try {
                        double d = std::stod(type::stripFloatSuffix(fconst->getValue()));
                        symbolTable.setGlobalInitializer(storageName, doubleBitsAsLong(d));
                    } catch (...) {
                        diagnostics.error("global floating initializer is not a constant", declarator.getContext());
                    }
                } else if (declarator.getInitializer()->evaluateConstant(initValue)) {
                    // Integer constant converted to floating: store as double bits.
                    symbolTable.setGlobalInitializer(storageName,
                            doubleBitsAsLong(static_cast<double>(initValue)));
                } else {
                    diagnostics.error("global floating initializer is not a constant expression", declarator.getContext());
                }
            } else if (declarator.getInitializer()->evaluateConstant(initValue)) {
                symbolTable.setGlobalInitializer(storageName, initValue);
            } else {
                // Peel outer (T) casts so void *p = (void *)&g is an address constant.
                ast::Expression* peeledInit = peelTypeCasts(declarator.getInitializer());
                // Address constant: &g, &s.m, &a[i], (T)&..., and ptr +/- n.
                AddressConstant addrConst;
                if (resolveAddressConstant(peeledInit, addrConst)) {
                    symbolTable.setGlobalAddressInitializer(storageName, addrConst.toOperand());
                } else if (auto* id = dynamic_cast<ast::IdentifierExpression*>(declarator.getInitializer());
                        id && symbolTable.hasFunction(id->getIdentifier())) {
                    // Function designator decays to pointer: void (*fp)(void) = f;
                    symbolTable.setGlobalAddressInitializer(storageName, id->getIdentifier());
                } else if (auto* list = dynamic_cast<ast::InitializerListExpression*>(declarator.getInitializer())) {
                    // File-scope brace/designated init -> multi-word .data
                    type::Type objectType = type;
                    type::Type completedType = type;
                    std::vector<std::string> words;
                    if (lowerToDataWords(objectType, list, symbolTable, completedType, words)) {
                        // Incomplete arrays may have been completed by the lowerer.
                        if (completedType.getSize() != type.getSize()) {
                            symbolTable.setGlobalType(storageName, completedType);
                            symbolTable.setLocalType(declarator.getName(), completedType);
                            declarator.setHolder(symbolTable.lookup(declarator.getName()));
                            type = completedType;
                        }
                        symbolTable.setGlobalMultiWordInitializer(storageName, std::move(words));
                    }
                } else {
                    diagnostics.error("global initializer is not a constant expression", declarator.getContext());
                }
            }
        } else if (declarator.hasInitializer() && !symbolTable.isAtFileScope()
                && declarator.getHolder()
                && declarator.getHolder()->getType().isAggregate()) {
            // Local aggregate / char-array-from-string -> per-field stores.
            type::Type holderType = declarator.getHolder()->getType();
            auto* initExpr = declarator.getInitializer();
            if (auto* strLit = dynamic_cast<ast::StringLiteralExpression*>(initExpr)) {
                if (holderType.isArray() && holderType.getArraySize() == 0) {
                    holderType = completeArrayTypeFromString(holderType, strLit);
                    symbolTable.setLocalType(declarator.getName(), holderType);
                    declarator.setHolder(symbolTable.lookup(declarator.getName()));
                }
            } else if (auto* list = dynamic_cast<ast::InitializerListExpression*>(initExpr)) {
                if (holderType.isArray() && holderType.getArraySize() == 0) {
                    holderType = completeArrayTypeFromList(holderType, list);
                    symbolTable.setLocalType(declarator.getName(), holderType);
                    declarator.setHolder(symbolTable.lookup(declarator.getName()));
                }
            }
            lowerToFieldInits(holderType, initExpr, symbolTable,
                    [&](ast::StructFieldInit init) {
                        declarator.addStructFieldInit(std::move(init));
                    });
        }
    } else {
        diagnostics.error(
                "symbol `" + declarator.getName() +
                        "` declaration conflicts with previous declaration on " +
                        to_string(symbolTable.lookup(declarator.getName()).getContext()),
                declarator.getContext());
    }
}

} // namespace semantic_analyzer
