#include "TokenStream.h"

#include "scanner/Scanner.h"
#include "scanner/Token.h"

namespace parser {

TokenStream::TokenStream(Scanner* scanner) :
    scanner { scanner },
    currentToken { new Token { this->scanner->nextToken() } }
{
}

TokenStream::~TokenStream() {
}

Token TokenStream::getCurrentToken() const {
	return (forgedToken ? *forgedToken : *currentToken);
}

Token TokenStream::nextToken() {
	if (forgedToken) {
        forgedToken.reset();
	} else {
		currentToken.reset(new Token { scanner->nextToken() });
	}
	return *currentToken;
}

void TokenStream::forgeToken(Token forgedToken) {
	this->forgedToken.reset(new Token { forgedToken });
}

bool TokenStream::currentTokenIsForged() const {
	return (forgedToken ? true : false);
}

} // namespace parser

