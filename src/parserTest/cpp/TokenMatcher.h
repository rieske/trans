#ifndef TOKENMATCHER_H_
#define TOKENMATCHER_H_

#include <string>
#include <tuple>

#include "gmock/gmock.h"

MATCHER_P(tokenMatches, expectedToken, std::string("a matching token [" +
                        testing::PrintToString(expectedToken.id) + ", " +
                        testing::PrintToString(expectedToken.lexeme) + ", {" +
                            testing::PrintToString(expectedToken.context.getSourceName()) +
                            ", " +
                            testing::PrintToString(expectedToken.context.getOffset()) +
                        "} ]")) {
    *result_listener << "actual is [" << arg.id << ", \"" << arg.lexeme << "\", {" << arg.context.getSourceName() << ", "
            << arg.context.getOffset() <<"} ]";
    return (arg.id == expectedToken.id) && (arg.lexeme == expectedToken.lexeme)
            && (arg.context.getOffset() == expectedToken.context.getOffset())
            && (arg.context.getSourceName() == expectedToken.context.getSourceName());
}

MATCHER_P2(tokenMatches, id, lexeme, std::string("a matching token [" +
                testing::PrintToString(id) + ", " + testing::PrintToString(lexeme) + "]")){
*result_listener << "actual is [" << arg.id << ", \"" << arg.lexeme << "\"]";
return (arg.id == id) && (arg.lexeme == lexeme);
}

#endif /* TOKENMATCHER_H_ */
