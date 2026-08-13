#ifndef LEXICALSESSION_H_
#define LEXICALSESSION_H_

#include "EnumConstantRegistry.h"
#include "ObjectTypeRegistry.h"
#include "TypedefRegistry.h"

namespace scanner {

// Not copyable: FA holds a raw pointer into typedefs.
struct LexicalSession {
    TypedefRegistry typedefs;
    ObjectTypeRegistry objects;
    EnumConstantRegistry enums;
    // Set when TokenFilter sees __attribute__((transparent_union)).
    // Consumed on the next finished declaration (typedef apply or ordinary discard).
    bool pendingTransparentUnion { false };

    bool consumePendingTransparentUnion() {
        const bool pending = pendingTransparentUnion;
        pendingTransparentUnion = false;
        return pending;
    }

    void enterBlock() {
        typedefs.pushIdentifierShadowScope();
        objects.pushScope();
        typedefs.flushPendingParameterShadows();
        objects.flushPending();
    }
    void leaveBlock() {
        typedefs.popIdentifierShadowScope();
        objects.popScope();
    }
    void endDeclarators() {
        typedefs.clearPendingParameterShadows();
        objects.clearPending();
    }

    LexicalSession() = default;
    LexicalSession(const LexicalSession&) = delete;
    LexicalSession& operator=(const LexicalSession&) = delete;
    LexicalSession(LexicalSession&&) = delete;
    LexicalSession& operator=(LexicalSession&&) = delete;
};

} // namespace scanner

#endif // LEXICALSESSION_H_
