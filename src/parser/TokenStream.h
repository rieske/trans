#ifndef TOKENSTREAM_H_
#define TOKENSTREAM_H_

#include <memory>

class Scanner;
class Token;

namespace parser {

class TokenStream {
public:
	TokenStream(Scanner* scanner);
	virtual ~TokenStream();

	Token getCurrentToken() const;
	Token nextToken();

	void forgeToken(Token forgedToken);
	bool currentTokenIsForged() const;
private:
	Scanner* scanner;

	std::unique_ptr<const Token> currentToken;
	std::unique_ptr<const Token> forgedToken;
};

}

#endif /* TOKENSTREAM_H_ */
