#ifndef _BASIC_TYPE_H_
#define _BASIC_TYPE_H_

namespace ast {

enum class BasicType {
    INTEGER, CHARACTER, FLOAT, // base type, can be pointed to

    VOID, FUNCTION, // can be pointed to

    LABEL
};

}

#endif /* _BASIC_TYPE_H_ */
