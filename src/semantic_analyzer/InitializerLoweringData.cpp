#include "InitializerLowering.h"
#include "InitializerLoweringInternal.h"

#include <sstream>
#include <vector>

#include "ConstantAddress.h"
#include "SymbolTable.h"
#include "ast/IdentifierExpression.h"
#include "ast/InitializerListExpression.h"
#include "ast/StringLiteralExpression.h"
#include "ast/TypeCast.h"
#include "types/ObjectAbi.h"
#include "util/StringLiteralDecode.h"
#include "Symbols.h"

namespace semantic_analyzer {


namespace {

struct DataWordSink : init_detail::AggregateInitSink {
    SymbolTable& symbolTable;
    std::vector<std::string>& words;
    int wordCount;

    DataWordSink(SymbolTable& st, std::vector<std::string>& w, int wc)
            : symbolTable { st }, words { w }, wordCount { wc } {
    }

    static bool parseNumericOperand(const std::string& s, unsigned long long& out) {
        if (s.empty()) {
            return false;
        }
        try {
            std::size_t idx = 0;
            if (s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
                out = std::stoull(s, &idx, 16);
            } else {
                long long signedVal = std::stoll(s, &idx, 10);
                out = static_cast<unsigned long long>(signedVal);
            }
            return idx == s.size();
        } catch (...) {
            return false;
        }
    }

    static std::string formatWord(unsigned long long v) {
        if (v > 0x7fffffffull) {
            std::ostringstream hex;
            hex << "0x" << std::hex << v;
            return hex.str();
        }
        return std::to_string(v);
    }

    void storeWord(int offsetBytes, const std::string& operand, int storeSizeBytes) {
        if (offsetBytes < 0 || storeSizeBytes <= 0) {
            return;
        }
        const int wi = type::object_abi::wordIndexAt(offsetBytes);
        if (wi < 0 || wi >= wordCount) {
            return;
        }
        if (storeSizeBytes >= type::object_abi::MACHINE_WORD_SIZE) {
            words[static_cast<std::size_t>(wi)] = operand;
            return;
        }
        unsigned long long storeVal = 0;
        if (!parseNumericOperand(operand, storeVal)) {
            words[static_cast<std::size_t>(wi)] = operand;
            return;
        }
        unsigned long long wordVal = 0;
        if (!parseNumericOperand(words[static_cast<std::size_t>(wi)], wordVal)) {
            wordVal = 0;
        }
        const int lane = offsetBytes % type::object_abi::MACHINE_WORD_SIZE;
        const int bits = storeSizeBytes * 8;
        const unsigned long long mask =
                bits >= 64 ? ~0ull : ((1ull << bits) - 1ull);
        wordVal &= ~(mask << (lane * 8));
        wordVal |= (storeVal & mask) << (lane * 8);
        words[static_cast<std::size_t>(wi)] = formatWord(wordVal);
    }

    std::string operandFromExpr(ast::Expression* value) {
        if (!value) {
            return "0";
        }
        value = peelTypeCasts(value);
        if (!value) {
            return "0";
        }
        long v = 0;
        if (value->evaluateConstant(v)) {
            if (static_cast<unsigned long long>(v) > 0x7fffffffull) {
                std::ostringstream hex;
                hex << "0x" << std::hex << static_cast<unsigned long long>(v);
                return hex.str();
            }
            return std::to_string(v);
        }
        if (auto* str = dynamic_cast<ast::StringLiteralExpression*>(value)) {
            const std::string& sym = str->getConstantSymbol();
            if (!sym.empty()) {
                return sym;
            }
        }
        AddressConstant addrConst;
        if (resolveAddressConstant(value, addrConst, [](ast::IdentifierExpression* id) {
                return defaultStorageLabel(id);
            })) {
            return addrConst.toOperand();
        }
        if (auto* id = dynamic_cast<ast::IdentifierExpression*>(value)) {
            if (symbolTable.hasFunction(id->getIdentifier())) {
                return id->getIdentifier();
            }
            if (value->hasResultSymbol()
                    && value->getResultSymbol()->getType().isArray()) {
                return defaultStorageLabel(id);
            }
        }
        return "0";
    }

    void placeScalar(int offsetBytes, const type::Type& storeType,
            ast::Expression* value) override {
        storeWord(offsetBytes, operandFromExpr(value), storeType.getSize());
    }

    bool placeStringArray(int offsetBytes, const type::Type& arrayType,
            ast::Expression* value) override {
        auto* strLit = dynamic_cast<ast::StringLiteralExpression*>(value);
        if (!strLit || !init_detail::isCharArrayType(arrayType)) {
            return false;
        }
        const auto bytes = util::decodeStringLiteralBytes(strLit->getValue());
        const int arrLen = arrayType.getArraySize();
        const int n = arrLen > 0 ? arrLen : static_cast<int>(bytes.size());
        for (int bi = 0; bi < n; ++bi) {
            unsigned char byte = (bi < static_cast<int>(bytes.size()))
                    ? bytes[static_cast<std::size_t>(bi)] : 0;
            storeWord(offsetBytes + bi,
                    std::to_string(static_cast<unsigned>(byte)), 1);
        }
        return true;
    }

    void placeAggregateCopy(int /*offsetBytes*/, const type::Type& /*storeType*/,
            ValueEntry* /*source*/) override {
        // File-scope path only folds constants / addresses, not live temps.
    }

    void onUnwritten(int /*offsetBytes*/, const type::Type& /*t*/) override {
        // Words pre-filled with "0".
    }
};

} // namespace

bool lowerToDataWords(type::Type objectType,
        ast::Expression* initializer,
        SymbolTable& symbolTable,
        type::Type& outObjectType,
        std::vector<std::string>& outWords) {
    auto* list = dynamic_cast<ast::InitializerListExpression*>(initializer);
    if (!list) {
        return false;
    }
    if (!objectType.isAggregate()) {
        return false;
    }

    if (objectType.isArray() && objectType.getArraySize() == 0) {
        objectType = completeArrayTypeFromList(objectType, list);
    }
    outObjectType = objectType;

    const int totalSize = objectType.getSize();
    const int wordCount = type::object_abi::dataWords(totalSize);
    if (wordCount <= 0) {
        return false;
    }

    std::vector<std::string> words(static_cast<std::size_t>(wordCount), "0");
    DataWordSink sink { symbolTable, words, wordCount };
    init_detail::walkAggregateInit(objectType, list, 0, sink);
    outWords = std::move(words);
    return true;
}

} // namespace semantic_analyzer
