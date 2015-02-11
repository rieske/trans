#ifndef FUNDAMENTALTYPE_H_
#define FUNDAMENTALTYPE_H_

namespace ast {

class FundamentalType {
public:
    virtual ~FundamentalType() = default;

    virtual FundamentalType* clone() const = 0;

protected:
    FundamentalType() = default;
    FundamentalType(const FundamentalType& rhs) = default;
    FundamentalType(FundamentalType&& rhs) = default;
    FundamentalType& operator=(const FundamentalType& rhs) = default;
    FundamentalType& operator=(FundamentalType&& rhs) = default;
};

}

#endif /* FUNDAMENTALTYPE_H_ */
