#ifndef LEXICALSESSION_H_
#define LEXICALSESSION_H_

#include "EnumConstantRegistry.h"
#include "ObjectTypeRegistry.h"
#include "TypedefRegistry.h"

#include <stack>

namespace scanner {

// Packed is a per-record-frame flag. Attributes are skipped as lookahead, so:
// after a struct/union token -> late_ (begin() inherits it);
// after '}' -> the open frame (spec has not reduced yet);
// after consume() -> late_ again (PE applyPacked).
// A prefix attribute is dropped.
struct RecordPacked {
    void noteStructOrUnionToken() { afterStructOrUnion_ = true; }

    void begin() {
        stack_.push(late_);
        late_ = false;
        afterStructOrUnion_ = false;
        justCompleted_ = false;
    }

    void notePacked() {
        if (afterStructOrUnion_ || justCompleted_) {
            late_ = true;
        } else if (!stack_.empty()) {
            stack_.top() = true;
        }
    }

    bool consume() {
        const bool value = stack_.top();
        stack_.pop();
        justCompleted_ = true;
        return value;
    }

    void abandon() {
        if (!stack_.empty()) {
            stack_.pop();
        }
        justCompleted_ = false;
        afterStructOrUnion_ = false;
    }

    bool takeLatePacked() {
        const bool value = late_;
        late_ = false;
        return value;
    }

private:
    std::stack<bool> stack_;
    bool late_ { false };
    bool afterStructOrUnion_ { false };
    bool justCompleted_ { false };
};

// transparent_union is a typedef leftover, not a record-frame attribute.
struct PendingTransparentUnion {
    void note() { pending_ = true; }

    bool consume() {
        const bool value = pending_;
        pending_ = false;
        return value;
    }

    void discard() { pending_ = false; }

private:
    bool pending_ { false };
};

// Not copyable: FA holds a raw pointer into typedefs.
struct LexicalSession {
    TypedefRegistry typedefs;
    ObjectTypeRegistry objects;
    EnumConstantRegistry enums;
    RecordPacked recordPacked;
    PendingTransparentUnion transparentUnion;

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
