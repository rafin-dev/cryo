#pragma once

#include <string>

namespace cryo::compiler {

    enum TokenType {
        NONE = 0,
        COMMENT,
        IDENTIFIER, // ...

        LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE, // ( ) { }
        COMMA, DOT, MINUS, PLUS, SLASH, ASTERISK, SEMICOLON, COLON, // , . - + / * ; :

        BANG, BANG_EQUAL, // ! !=
        EQUAL, EQUAL_EQUAL, // = ==
        GREATER, GREATER_EQUAL, // > >=
        LESS, LESS_EQUAL, // < <=
        AND, AND_AND, // & &&
        OR, OR_OR, // | ||

        RETURN_TYPE, // ->

        STRING, INT, FLOAT, // "*" [1/2/3...] *.*

        // Self-explanatory
        FN, LET, IF, ELSE, TRUE, FALSE, FOR, WHILE,
        CLASS, THIS, RETURN, PRINT,
    };

    struct Token {
        std::string lexeme;
        TokenType Type = NONE;
    };

}
