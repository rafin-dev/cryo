#pragma once
#include "shared/Error.h"
#include "Token.h"

#include <cstdint>

namespace cryo::compiler {

    class Lexer {
    public:
        Lexer(std::string source, std::filesystem::path file);

        [[nodiscard]]
        std::pair<std::vector<Token>, ErrorQueue> get_tokens();

    private:
        [[nodiscard]]
        std::variant<Token, uint32_t> scan_token(uint32_t current_char);
        [[nodiscard]]
        std::variant<Token, uint32_t> string_token(uint32_t start);
        [[nodiscard]]
        std::variant<Token, uint32_t> numeric_token(uint32_t start);
        [[nodiscard]]
        std::variant<Token, uint32_t> identifier_token(uint32_t start);

        void push_error(std::string_view error_code, const std::string& error_message, std::string_view highlight);

        std::filesystem::path m_File;
        std::string m_SrcCode;
        ErrorQueue m_ErrorQueue;
        uint32_t m_CurrentLine = 1;

        static std::unordered_map<std::string, TokenType> s_Keywords;
    };

}