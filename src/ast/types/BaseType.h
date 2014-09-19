#ifndef BASE_TYPE_H_
#define BASE_TYPE_H_

namespace ast {

class BaseType {
public:
    static const BaseType CHARACTER;
    static const BaseType INTEGER;
    static const BaseType FLOAT;

    static BaseType promote(BaseType type1, BaseType type2);

    bool isInteger() const noexcept;

    bool operator==(const BaseType& rhs) const noexcept;
    bool operator!=(const BaseType& rhs) const noexcept;
private:
    BaseType(int sizeOrder);

    const int sizeOrder;
};

}

#endif /* BASE_TYPE_H_ */
