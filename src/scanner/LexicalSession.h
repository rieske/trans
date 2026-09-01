#ifndef LEXICALSESSION_H_
#define LEXICALSESSION_H_

#include "EnumConstantRegistry.h"
#include "IdentifierTable.h"

#include <stack>
#include <vector>

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

// Compound `{` hops names and enumerators. Record `{` hops names only.
// Enum `{` is not a C block; enumerators stay on the enclosing frame.
enum class BraceFrame { Block, Record, EnumBody };

// Not copyable: FA holds a raw pointer to the session.
struct LexicalSession {
    IdentifierTable names;
    EnumConstantRegistry enums;
    RecordPacked recordPacked;
    PendingTransparentUnion transparentUnion;

    bool isTypedef(const std::string& name) const { return names.has(name); }
    bool isEnumerator(const std::string& name) const { return enums.contains(name); }
    bool lookupEnumerator(const std::string& name, type::IntegerConstant& value) const {
        return enums.lookup(name, value);
    }

    void enterBlock() {
        names.enterScope();
        enums.enterScope();
    }
    void leaveBlock() {
        names.leaveScope();
        enums.leaveScope();
    }
    void enterRecord() {
        names.enterScope();
    }
    void leaveRecord() {
        names.leaveScope();
    }
    void openBrace(BraceFrame kind) {
        braces_.push_back(kind);
        if (kind == BraceFrame::Block) {
            enterBlock();
        } else if (kind == BraceFrame::Record) {
            enterRecord();
        }
    }
    void closeBrace() {
        if (braces_.empty()) {
            leaveBlock();
            return;
        }
        const BraceFrame kind = braces_.back();
        braces_.pop_back();
        if (kind == BraceFrame::Block) {
            leaveBlock();
        } else if (kind == BraceFrame::Record) {
            leaveRecord();
        }
    }
    void endDeclarators() {
        names.clearPendingParameterShadows();
        names.clearPendingObjects();
    }

    LexicalSession() {
        names.add("_Float32", type::floating());
        names.add("_Float64", type::doubleFloating());
        names.add("_Float128", type::doubleFloating());
        names.add("_Float32x", type::floating());
        names.add("_Float64x", type::doubleFloating());
    }
    LexicalSession(const LexicalSession&) = delete;
    LexicalSession& operator=(const LexicalSession&) = delete;
    LexicalSession(LexicalSession&&) = delete;
    LexicalSession& operator=(LexicalSession&&) = delete;

private:
    std::vector<BraceFrame> braces_;
};

} // namespace scanner

#endif // LEXICALSESSION_H_
