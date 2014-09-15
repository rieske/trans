#ifndef _BASIC_TYPE_H_
#define _BASIC_TYPE_H_

namespace ast {

enum class BasicType {
    INTEGER, CHARACTER, FLOAT, VOID, LABEL, FUNCTION
};

class NumberType;

class Type {
public:
    virtual ~Type() {}

    virtual bool isNumber() const = 0;

};

class NumberType: public Type {
public:
    bool isNumber() const override { return true; }
    NumberType(int width): width {width} {}

private:
    int width;
};

}

#endif /* _BASIC_TYPE_H_ */
