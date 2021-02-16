#ifndef TOKENSTREAM_H_
#define TOKENSTREAM_H_

#include "scanner/Scanner.h"
#include <memory>

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

} // namespace parser

#endif // TOKENSTREAM_H_
