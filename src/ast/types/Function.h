#ifndef FUNCTION_H_
#define FUNCTION_H_

#include "BaseType.h"

namespace ast {

class Function: public BaseType {
public:
    Function();
    virtual ~Function();
};

} /* namespace ast */

#endif /* FUNCTION_H_ */
