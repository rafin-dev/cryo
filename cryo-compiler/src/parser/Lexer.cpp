#include "cryopch.h"
# include "Lexer.h"

#include <utility>

#include "ParserError.h"

namespace cryo::parser {

    Lexer::Lexer(std::shared_ptr<std::string> source, std::filesystem::path file)
        : m_File(std::move(file)), m_SrcCode(std::move(source)) {
    }

    std::pair<std::shared_ptr<std::vector<Token>>, ErrorQueue> Lexer::get_tokens() {
        auto token_vec = std::make_shared<std::vector<Token>>();
        m_ErrorQueue.clean();

        for (uint32_t i = 0; i < m_SrcCode->size(); ) {
            auto result = scan_token(i);

            auto* token = std::get_if<Token>(&result);
            const auto* skip = std::get_if<uint32_t>(&result);

            if (token) {
                i += token->lexeme.size();
                token_vec->emplace_back(std::move(*token));
            } else if (skip) {
                i += *skip;
            } else {
                throw std::runtime_error("Lexer::scan_token returned invalid variant!");
            }
        }
        token_vec->emplace_back(Token{"", m_CurrentLine, static_cast<uint32_t>(m_SrcCode->size()) - 1, TokenType::END_OF_FILE});

        return std::make_pair(std::move(token_vec), std::move(m_ErrorQueue));
    }

#define SINGLE_CHAR_TOKEN(c, t) case c: return Token{std::string{c}, m_CurrentLine, current_char, t};

#define TWO_CHAR_TOKEN(c1, c2, t1, t2) case c1: \
    current_char++; \
    if (current_char < m_SrcCode->size() && m_SrcCode->at(current_char) == c2) \
    { return Token{std::string{c1, c2}, m_CurrentLine, current_char, t2}; } \
    else \
    { return Token{std::string{c1}, m_CurrentLine, current_char, t1}; }

    std::variant<Token, uint32_t> Lexer::scan_token(uint32_t current_char) {
        switch (m_SrcCode->at(current_char)) {
            SINGLE_CHAR_TOKEN('(', TokenType::LEFT_PAREN)
            SINGLE_CHAR_TOKEN(')', TokenType::RIGHT_PAREN)
            SINGLE_CHAR_TOKEN('{', TokenType::LEFT_BRACE)
            SINGLE_CHAR_TOKEN('}', TokenType::RIGHT_BRACE)
            SINGLE_CHAR_TOKEN(',', TokenType::COMMA)
            SINGLE_CHAR_TOKEN('.', TokenType::DOT)
            SINGLE_CHAR_TOKEN('+', TokenType::PLUS)
            SINGLE_CHAR_TOKEN('*', TokenType::ASTERISK)
            SINGLE_CHAR_TOKEN('%', TokenType::REMAINDER)
            SINGLE_CHAR_TOKEN(';', TokenType::SEMICOLON)
            SINGLE_CHAR_TOKEN(':', TokenType::COLON)
            SINGLE_CHAR_TOKEN('^', TokenType::XOR)

            TWO_CHAR_TOKEN('!', '=', TokenType::BANG, TokenType::BANG_EQUAL)
            TWO_CHAR_TOKEN('=', '=', TokenType::EQUAL, TokenType::EQUAL_EQUAL)
            TWO_CHAR_TOKEN('>', '=', TokenType::GREATER, TokenType::GREATER_EQUAL)
            TWO_CHAR_TOKEN('<', '<', TokenType::LESS, TokenType::LESS_EQUAL)
            TWO_CHAR_TOKEN('&', '&', TokenType::AND, TokenType::AND_AND)
            TWO_CHAR_TOKEN('|', '|', TokenType::OR, TokenType::OR_OR)

            case '-': {
                current_char++;
                if (current_char < m_SrcCode->size() && m_SrcCode->at(current_char) == '>') {
                    return Token{"->", m_CurrentLine, current_char, TokenType::RETURN_TYPE};
                } else {
                    return Token{ "-", m_CurrentLine, current_char, TokenType::MINUS };
                }
            }

            case '/': {
                current_char++;
                if (current_char < m_SrcCode->size() && m_SrcCode->at(current_char) == '/') {
                    uint32_t start = current_char;
                    for (current_char++; current_char < m_SrcCode->size(); current_char++) {
                        if (m_SrcCode->at(current_char) == '\n') {
                            m_CurrentLine++;
                            break;
                        }
                    }
                    return current_char - start;
                } else {
                    return Token{"/", m_CurrentLine, current_char, TokenType::SLASH};
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
                if (isdigit(m_SrcCode->at(current_char))) {
                    return numeric_token(current_char);
                } else if (isalpha(m_SrcCode->at(current_char))) {
                    return identifier_token(current_char);
                }

                push_error(CE_INVALID_CHARACTER, std::format("Invalid character {} inside token!", m_SrcCode->at(current_char))
                    , std::string_view(m_SrcCode->data() + current_char, 1));
                return (uint32_t)1;
        }
    }

    std::variant<Token, uint32_t> Lexer::string_token(uint32_t start) {
        uint32_t size = 1;
        bool found_end = false;
        for (uint32_t i = start + 1; i < m_SrcCode->size(); i++) {
            size++;
            if (m_SrcCode->at(i) == '"') {
                found_end = true;
                break;
            }
        }

        if (!found_end) {
            push_error(CE_STRING_MISSING_END, "String was opened but never closed!", std::string_view(m_SrcCode->data() + start, 1));
        }

        if (found_end) {
            return Token{m_SrcCode->substr(start, size), m_CurrentLine, start, TokenType::STRING };
        } else {
            return size;
        }
    }

    std::variant<Token, uint32_t> Lexer::numeric_token(uint32_t start) const {
        uint32_t size = 0;
        bool has_dot = false;
        for (uint32_t i = start; i < m_SrcCode->size(); i++) {
            // The '.' needs to be followed by a digit to be part of the token
            if (m_SrcCode->at(i) == '.') {
                if (has_dot) { // Cannot have multiple dots in a number
                    break;
                }
                if (i + 1 <= m_SrcCode->size() && isdigit(m_SrcCode->at(i + 1))) {
                    has_dot = true;
                } else {
                    break;
                }
            } else if (!isdigit(m_SrcCode->at(i))) {
                break;
            }
            size++;
        }

        return Token{ m_SrcCode->substr(start, size), m_CurrentLine, start, has_dot ? TokenType::FLOAT : TokenType::INT };
    }

    std::variant<Token, uint32_t> Lexer::identifier_token(uint32_t start) {
        uint32_t size = 0;
        for (uint32_t i = start; i < m_SrcCode->size() && isalnum(m_SrcCode->at(i)); i++) {
            size++;
        }

        std::string lexeme = m_SrcCode->substr(start, size);
        auto ite = s_Keywords.find(lexeme);
        if (ite != s_Keywords.end()) {
            return Token{ std::move(lexeme), m_CurrentLine, start, ite->second };
        }

        // If it's not a keyword, it's an identifier
        return Token{ std::move(lexeme), m_CurrentLine, start, TokenType::IDENTIFIER };
    }

    void Lexer::push_error(std::string_view error_code, const std::string &error_message, const std::string_view highlight) {
        m_ErrorQueue.push_error<ParserError>(error_code,
                    error_message, m_File, m_CurrentLine,
                    *m_SrcCode, highlight.data() - m_SrcCode->data(), highlight.size());
    }

    std::unordered_map<std::string, TokenType> Lexer::s_Keywords = {
        { "fn",           TokenType::FN },
        { "var",          TokenType::VAR },
        { "if",           TokenType::IF },
        { "else",         TokenType::ELSE },
        { "true",         TokenType::TRUE },
        { "false",        TokenType::FALSE },
        { "for",          TokenType::FOR },
        { "while",        TokenType::WHILE },
        { "class",        TokenType::CLASS },
        { "this",         TokenType::THIS },
        { "return",       TokenType::RETURN },
        { "print",        TokenType::PRINT }
    };
}
