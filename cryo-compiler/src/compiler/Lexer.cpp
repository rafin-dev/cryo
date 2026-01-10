#include "cryopch.h"
# include "Lexer.h"

#include <utility>

namespace cryo::compiler {

    Lexer::Lexer(std::string source)
        : m_SrcCode(std::move(source)) {
    }

    std::pair<std::vector<Token>, ErrorQueue> Lexer::get_tokens() {
        std::vector<Token> token_vec;
        m_ErrorQueue.clean();

        for (uint32_t i = 0; i < m_SrcCode.size(); ) {
            Token token = scan_token(i);

            i += token.lexeme.size();

            if (token.Type != NONE) {
                token_vec.emplace_back(std::move(token));
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

    Token Lexer::scan_token(uint32_t current_char) {
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
                    // TODO: errors
                }
            }

            case '/': {
                char *start = m_SrcCode.data() + current_char;
                current_char++;
                if (current_char < m_SrcCode.size() && m_SrcCode[current_char] == '/') {
                    for (current_char++; current_char < m_SrcCode.size() && m_SrcCode[current_char] != '\n';
                         current_char++);
                    char *end = m_SrcCode.data() + current_char;
                    m_CurrentLine++;
                    return Token{std::string(start, end), NONE};
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
                return Token{" ", NONE};

            default:
                if (isdigit(m_SrcCode[current_char])) {
                    return numeric_token(current_char);
                } else if (isalpha(m_SrcCode[current_char])) {
                    return identifier_token(current_char);
                }

                // TODO: Errors
                return Token{};
        }
        return Token{};
    }

    Token Lexer::string_token(uint32_t start) {
        uint32_t size = 0;
        bool found_end = false;
        for (uint32_t i = start + 1; i < m_SrcCode.size(); i++) {
            if (m_SrcCode[i] == '"') {
                found_end = true;
                break;
            }
            size++;
        }

        if (!found_end) {
            // TODO: STRING MISSING END ERROR
        }

        return Token{ m_SrcCode.substr(start + 1, size), STRING };
    }

    Token Lexer::numeric_token(uint32_t start) {
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

    Token Lexer::identifier_token(uint32_t start) {
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
