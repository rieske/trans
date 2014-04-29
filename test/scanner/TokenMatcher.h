#ifndef TOKENMATCHER_H_
#define TOKENMATCHER_H_

#include "gmock/gmock.h"

MATCHER_P(tokenMatches, expectedToken, std::string("a matching token [" +
				testing::PrintToString(expectedToken.getId()) + ", " + testing::PrintToString(expectedToken.getLexeme()) + "]")){
	*result_listener << "actual is [" << arg.getId() << ", \"" << arg.getLexeme() << "\"]";
	return (arg.getId() == expectedToken.getId()) && (arg.getLexeme() == expectedToken.getLexeme());
}

MATCHER_P2(tokenMatches, id, lexeme, std::string("a matching token [" +
				testing::PrintToString(id) + ", " + testing::PrintToString(lexeme) + "]")){
	*result_listener << "actual is [" << arg.getId() << ", \"" << arg.getLexeme() << "\"]";
	return (arg.getId() == id) && (arg.getLexeme() == lexeme);
}

#endif /* TOKENMATCHER_H_ */
