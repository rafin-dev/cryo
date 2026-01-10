#pragma once
#include "shared/Error.h"
#include "Token.h"

#include <cstdint>

namespace cryo::compiler {

    class Lexer {
    public:
        Lexer(std::string source);

        std::pair<std::vector<Token>, ErrorQueue> get_tokens();

    private:
        [[nodiscard]]
        Token scan_token(uint32_t current_char);
        [[nodiscard]]
        Token string_token(uint32_t start);
        [[nodiscard]]
        Token numeric_token(uint32_t start);
        [[nodiscard]]
        Token identifier_token(uint32_t start);

        std::string m_SrcCode;
        ErrorQueue m_ErrorQueue;
        uint32_t m_CurrentLine = 1;

        static std::unordered_map<std::string, TokenType> s_Keywords;
    };

}