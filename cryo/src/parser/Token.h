#pragma once

#include <string>

namespace cryo::parser {

    enum class TokenType {
        IDENTIFIER, // ...

        LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE, // ( ) { }
        COMMA, DOT, MINUS, PLUS, SLASH, ASTERISK, REMAINDER, SEMICOLON, COLON, // , . - + / * ; :

        BANG, BANG_EQUAL, // ! !=
        EQUAL, EQUAL_EQUAL, // = ==
        GREATER, GREATER_EQUAL, // > >=
        LESS, LESS_EQUAL, // < <=
        AND, AND_AND, // & &&
        OR, OR_OR, // | ||

        RETURN_TYPE, // ->

        STRING, INT, FLOAT, // "*" [1/2/3...] *.*

        // Self-explanatory
        FN, VAR, IF, ELSE, TRUE, FALSE, FOR, WHILE, LOOP,
        CLASS, THIS, RETURN, CONTINUE, BREAK,

        END_OF_FILE
    };

    inline std::string_view TokenType_to_string(const TokenType type) {
        switch (type) {
            case TokenType::IDENTIFIER: return "Identifier";
            case TokenType::LEFT_PAREN: return "(";
            case TokenType::RIGHT_PAREN: return ")";
            case TokenType::LEFT_BRACE: return "{";
            case TokenType::RIGHT_BRACE: return "}";
            case TokenType::COMMA: return ",";
            case TokenType::DOT: return ".";
            case TokenType::MINUS: return "-";
            case TokenType::PLUS: return "+";
            case TokenType::SLASH: return "/";
            case TokenType::ASTERISK: return "*";
            case TokenType::REMAINDER: return "%";
            case TokenType::SEMICOLON: return ";";
            case TokenType::COLON: return ":";
            case TokenType::BANG: return "!";
            case TokenType::BANG_EQUAL: return "!=";
            case TokenType::EQUAL: return "=";
            case TokenType::EQUAL_EQUAL: return "==";
            case TokenType::GREATER: return ">";
            case TokenType::GREATER_EQUAL: return ">=";
            case TokenType::LESS: return "<";
            case TokenType::LESS_EQUAL: return "<=";
            case TokenType::AND_AND: return "&&";
            case TokenType::OR_OR: return "||";
            case TokenType::RETURN_TYPE: return "->";
            case TokenType::STRING: return "String";
            case TokenType::INT: return "Integer";
            case TokenType::FLOAT: return "Float";
            case TokenType::FN: return "fn";
            case TokenType::VAR: return "var";
            case TokenType::IF: return "if";
            case TokenType::ELSE: return "else";
            case TokenType::TRUE: return "true";
            case TokenType::FALSE: return "false";
            case TokenType::FOR: return "for";
            case TokenType::WHILE: return "while";
            case TokenType::LOOP: return "loop";
            case TokenType::CLASS: return "class";
            case TokenType::THIS: return "this";
            case TokenType::RETURN: return "return";
            case TokenType::CONTINUE: return "continue";
            case TokenType::BREAK: return "break";
            case TokenType::END_OF_FILE: return "EOF";

            default:
                throw std::runtime_error("Invalid TokenType!");
                return "";
        }
    }

    struct Token {
        std::string lexeme;
        uint32_t LineNumber;
        uint32_t IndexFromSource;
        TokenType Type;
    };

}
