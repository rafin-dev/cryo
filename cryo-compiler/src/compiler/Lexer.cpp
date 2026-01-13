#include "cryopch.h"
# include "Lexer.h"

#include <utility>

#include "CompilerError.h"

namespace cryo::compiler {

    Lexer::Lexer(std::string source, std::filesystem::path file)
        : m_File(std::move(file)), m_SrcCode(std::move(source)) {
    }

    std::pair<std::vector<Token>, ErrorQueue> Lexer::get_tokens() {
        std::vector<Token> token_vec;
        m_ErrorQueue.clean();

        for (uint32_t i = 0; i < m_SrcCode.size(); ) {
            auto result = scan_token(i);

            auto* token = std::get_if<Token>(&result);
            const auto* skip = std::get_if<uint32_t>(&result);

            if (token) {
                i += token->lexeme.size();
                token_vec.emplace_back(std::move(*token));
            } else if (skip) {
                i += *skip;
            } else {
                throw std::runtime_error("Lexer::scan_token returned invalid variant!");
            }
        }

        return std::make_pair(std::move(token_vec), std::move(m_ErrorQueue));
    }

#define SINGLE_CHAR_TOKEN(c, t) case c: return Token{std::string{c}, t};

#define TWO_CHAR_TOKEN(c1, c2, t1, t2) case c1: \
    current_char++; \
    if (current_char < m_SrcCode.size() && m_SrcCode[current_char] == c2) \
    { return Token{std::string{c1, c2}, t2}; } \
    else \
    { return Token{std::string{c1}, t1}; }

    std::variant<Token, uint32_t> Lexer::scan_token(uint32_t current_char) {
        switch (m_SrcCode[current_char]) {
            SINGLE_CHAR_TOKEN('(', LEFT_PAREN)
            SINGLE_CHAR_TOKEN(')', RIGHT_PAREN)
            SINGLE_CHAR_TOKEN('{', LEFT_BRACE)
            SINGLE_CHAR_TOKEN('}', RIGHT_BRACE)
            SINGLE_CHAR_TOKEN(',', COMMA)
            SINGLE_CHAR_TOKEN('.', DOT)
            SINGLE_CHAR_TOKEN('+', PLUS)
            SINGLE_CHAR_TOKEN('*', ASTERISK)
            SINGLE_CHAR_TOKEN(';', SEMICOLON)
            SINGLE_CHAR_TOKEN(':', COLON)

            TWO_CHAR_TOKEN('!', '=', BANG, BANG_EQUAL)
            TWO_CHAR_TOKEN('=', '=', EQUAL, EQUAL_EQUAL)
            TWO_CHAR_TOKEN('>', '=', GREATER, GREATER_EQUAL)
            TWO_CHAR_TOKEN('<', '<', LESS, LESS_EQUAL)
            TWO_CHAR_TOKEN('&', '&', AND, AND_AND)
            TWO_CHAR_TOKEN('|', '|', OR, OR_OR)

            case '-': {
                current_char++;
                if (current_char < m_SrcCode.size() && m_SrcCode[current_char] == '>') {
                    return Token{"->", RETURN_TYPE};
                } else {
                    return Token{ "-", MINUS };
                }
            }

            case '/': {
                current_char++;
                if (current_char < m_SrcCode.size() && m_SrcCode[current_char] == '/') {
                    uint32_t start = current_char;
                    for (current_char++; current_char < m_SrcCode.size(); current_char++) {
                        if (m_SrcCode[current_char] == '\n') {
                            m_CurrentLine++;
                            break;
                        }
                    }
                    return current_char - start;
                } else {
                    return Token{"/", SLASH};
                }
            }

            case '"': {
                return string_token(current_char);
            }

            case '\n':
                m_CurrentLine++;
            case ' ':
            case '\r':
            case '\t':
                return (uint32_t)1;

            default:
                if (isdigit(m_SrcCode[current_char])) {
                    return numeric_token(current_char);
                } else if (isalpha(m_SrcCode[current_char])) {
                    return identifier_token(current_char);
                }

                push_error(CE_INVALID_CHARACTER, std::format("Invalid character {} inside token!", m_SrcCode[current_char])
                    , std::string_view(m_SrcCode.data() + current_char, 1));
                return (uint32_t)1;
        }
    }

    std::variant<Token, uint32_t> Lexer::string_token(uint32_t start) {
        uint32_t size = 1;
        bool found_end = false;
        for (uint32_t i = start + 1; i < m_SrcCode.size(); i++) {
            size++;
            if (m_SrcCode[i] == '"') {
                found_end = true;
                break;
            }
        }

        if (!found_end) {
            push_error(CE_STRING_MISSING_END, "String was opened but never closed!", std::string_view(m_SrcCode.data() + start, 1));
        }

        if (found_end) {
            return Token{m_SrcCode.substr(start, size), STRING };
        } else {
            return size;
        }
    }

    std::variant<Token, uint32_t> Lexer::numeric_token(uint32_t start) {
        uint32_t size = 0;
        bool has_dot = false;
        for (uint32_t i = start; i < m_SrcCode.size(); i++) {
            // The '.' needs to be followed by a digit to be part of the token
            if (m_SrcCode[i] == '.') {
                if (has_dot) { // Cannot have multiple dots in a number
                    break;
                }
                if (i + 1 <= m_SrcCode.size() && isdigit(m_SrcCode[i + 1])) {
                    has_dot = true;
                } else {
                    break;
                }
            } else if (!isdigit(m_SrcCode[i])) {
                break;
            }
            size++;
        }

        return Token{ m_SrcCode.substr(start, size), has_dot ? FLOAT : INT };
    }

    std::variant<Token, uint32_t> Lexer::identifier_token(uint32_t start) {
        uint32_t size = 0;
        for (uint32_t i = start; i < m_SrcCode.size() && isalnum(m_SrcCode[i]); i++) {
            size++;
        }

        std::string lexeme = m_SrcCode.substr(start, size);
        auto ite = s_Keywords.find(lexeme);
        if (ite != s_Keywords.end()) {
            return Token{ std::move(lexeme), ite->second };
        }

        // If it's not a keyword, it's an identifier
        return Token{ std::move(lexeme), IDENTIFIER };
    }

    void Lexer::push_error(std::string_view error_code, const std::string &error_message, std::string_view highlight) {
        m_ErrorQueue.push_error<CompilerError>(error_code,
                    error_message, m_File, m_CurrentLine,
                    m_SrcCode, highlight.data() - m_SrcCode.data(), highlight.size());
    }

    std::unordered_map<std::string, TokenType> Lexer::s_Keywords = {
        { "fn", FN },
        { "let", LET },
        { "if", IF },
        { "else", ELSE },
        { "true", TRUE },
        { "false", FALSE },
        { "for", FOR },
        { "while", WHILE },
        { "class", CLASS },
        { "this", THIS },
        { "return", RETURN },
        { "print", PRINT }
    };
}
