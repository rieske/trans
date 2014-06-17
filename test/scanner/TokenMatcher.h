#ifndef TOKENMATCHER_H_
#define TOKENMATCHER_H_

#include "gmock/gmock.h"

MATCHER_P(tokenMatches, expectedToken, std::string("a matching token [" +
				testing::PrintToString(expectedToken.id) + ", " + testing::PrintToString(expectedToken.lexeme) + ", " + testing::PrintToString(expectedToken.line) + "]")){
	*result_listener << "actual is [" << arg.id << ", \"" << arg.lexeme << "\", " << arg.line <<"]";
	return (arg.id == expectedToken.id) && (arg.lexeme == expectedToken.lexeme) && (arg.line == expectedToken.line);
}

MATCHER_P2(tokenMatches, id, lexeme, std::string("a matching token [" +
				testing::PrintToString(id) + ", " + testing::PrintToString(lexeme) + "]")){
	*result_listener << "actual is [" << arg.id << ", \"" << arg.lexeme << "\"]";
	return (arg.id == id) && (arg.lexeme == lexeme);
}

#endif /* TOKENMATCHER_H_ */
