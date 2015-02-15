#ifndef FUNDAMENTALTYPE_H_
#define FUNDAMENTALTYPE_H_

#include <memory>
#include <string>

namespace ast {

class FundamentalType {
public:
    virtual ~FundamentalType() = default;

    virtual FundamentalType* clone() const = 0;

    virtual std::string toString() const = 0;

    bool canConvertTo(const FundamentalType& otherType) const;

    virtual bool isVoid() const;
    virtual bool isPointer() const;
    virtual bool isNumeric() const;
    virtual std::unique_ptr<FundamentalType> dereference() const;

protected:
    FundamentalType() = default;
    FundamentalType(const FundamentalType& rhs) = default;
    FundamentalType(FundamentalType&& rhs) = default;
    FundamentalType& operator=(const FundamentalType& rhs) = default;
    FundamentalType& operator=(FundamentalType&& rhs) = default;
};

}

#endif /* FUNDAMENTALTYPE_H_ */
