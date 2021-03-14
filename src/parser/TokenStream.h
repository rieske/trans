#ifndef TOKENSTREAM_H_
#define TOKENSTREAM_H_

#include "scanner/Scanner.h"
#include <memory>
#include <functional>

namespace parser {

class TokenStream {
public:
	TokenStream(std::function<scanner::Token()> scanner);

    scanner::Token getCurrentToken() const;
    scanner::Token nextToken();

	void forgeToken(scanner::Token forgedToken);
	bool currentTokenIsForged() const;
private:
    std::function<scanner::Token()> scanner;

	std::unique_ptr<const scanner::Token> currentToken;
	std::unique_ptr<const scanner::Token> forgedToken;
};

} // namespace parser

#endif // TOKENSTREAM_H_
