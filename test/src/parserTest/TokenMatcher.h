#ifndef TOKENMATCHER_H_
#define TOKENMATCHER_H_

#include <string>
#include <string_view>
#include <tuple>

#include "gmock/gmock.h"
#include "scanner/Token.h"

inline std::string_view tokenKindName(const scanner::Token& t) {
    using scanner::TokenClass;
    switch (t.cls) {
    case TokenClass::Id:
        return "id";
    case TokenClass::TypedefName:
        return "typedef_name";
    case TokenClass::String:
        return "string";
    case TokenClass::IntConst:
        return "int_const";
    case TokenClass::FloatConst:
        return "float_const";
    case TokenClass::CharConst:
        return "char_const";
    case TokenClass::End:
        return scanner::Token::END;
    case TokenClass::Lexeme:
        return t.lexeme;
    }
    return t.lexeme;
}

MATCHER_P(tokenMatches, expectedToken, std::string("a matching token [" +
                        testing::PrintToString(std::string { tokenKindName(expectedToken) }) + ", " +
                        testing::PrintToString(expectedToken.lexeme) + ", {" +
                            testing::PrintToString(expectedToken.context.getSourceName()) +
                            ", " +
                            testing::PrintToString(expectedToken.context.getOffset()) +
                        "} ]")) {
    *result_listener << "actual is [" << tokenKindName(arg) << ", \"" << arg.lexeme << "\", {" << arg.context.getSourceName() << ", "
            << arg.context.getOffset() <<"} ]";
    return arg.cls == expectedToken.cls && (arg.lexeme == expectedToken.lexeme)
            && (arg.context.getOffset() == expectedToken.context.getOffset())
            && (arg.context.getSourceName() == expectedToken.context.getSourceName());
}

MATCHER_P2(tokenMatches, id, lexeme, std::string("a matching token [" +
                testing::PrintToString(id) + ", " + testing::PrintToString(lexeme) + "]")){
*result_listener << "actual is [" << tokenKindName(arg) << ", \"" << arg.lexeme << "\"]";
return arg.cls == scanner::tokenClassFromName(id) && (arg.lexeme == lexeme);
}

#endif /* TOKENMATCHER_H_ */
