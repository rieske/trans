#ifndef TOKENSTREAM_H_
#define TOKENSTREAM_H_

#include "scanner/Scanner.h"
#include <memory>

namespace parser {

class TokenStream {
public:
	TokenStream(scanner::Scanner* scanner);
	virtual ~TokenStream();

    scanner::Token getCurrentToken() const;
    scanner::Token nextToken();

	void forgeToken(scanner::Token forgedToken);
	bool currentTokenIsForged() const;
private:
    scanner::Scanner* scanner;

	std::unique_ptr<const scanner::Token> currentToken;
	std::unique_ptr<const scanner::Token> forgedToken;
};

} // namespace parser

#endif // TOKENSTREAM_H_
