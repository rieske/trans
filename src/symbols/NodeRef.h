#ifndef SYMBOLS_NODEREF_H_
#define SYMBOLS_NODEREF_H_

// Opaque AST node identity for AnnotationStore keys and plan children.
// symbols does not depend on ast; cast with as<T>() only in SA/codegen.

#include <cstddef>
#include <functional>

namespace symbols {

class NodeRef {
public:
    NodeRef() = default;
    template <typename T>
    NodeRef(const T* node) : ptr_ { static_cast<const void*>(node) } {}

    explicit operator bool() const { return ptr_ != nullptr; }
    bool empty() const { return ptr_ == nullptr; }
    const void* get() const { return ptr_; }

    bool operator==(NodeRef o) const { return ptr_ == o.ptr_; }
    bool operator!=(NodeRef o) const { return ptr_ != o.ptr_; }

    template <typename T>
    T* as() const {
        return static_cast<T*>(const_cast<void*>(ptr_));
    }

private:
    const void* ptr_ { nullptr };
};

struct NodeRefHash {
    std::size_t operator()(NodeRef n) const noexcept {
        return std::hash<const void*> {}(n.get());
    }
};

template <typename T, typename Variant>
inline const T* get_if(const Variant* plan) {
    return plan ? std::get_if<T>(plan) : nullptr;
}

template <typename T, typename Variant>
inline const T* get_if(const Variant& plan) {
    return std::get_if<T>(&plan);
}

} // namespace symbols

#endif // SYMBOLS_NODEREF_H_
