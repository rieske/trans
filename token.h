#ifndef _TOKEN_H_
#define _TOKEN_H_

#include <string>

/**
 * Struktūra, apibūdinanti leksemą
 **/

typedef struct tag_token
{
    unsigned    type;
    std::string value;

    unsigned    line;          // eilutė, iš kurios paimta leksema
} Token;

#endif // _TOKEN_H_
