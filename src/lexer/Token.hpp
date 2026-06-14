#pragma once

#include <string>

enum class TokenType
{
    // Keywords
    LET,
    CONST,
    FUNCTION,
    IF,
    ELSE,
    DO,
    FOR,
    WHILE,
    SWITCH,
    CASE,
    DEFAULT,
    BREAK,
    RETURN,
    TRY,
    CATCH,
    FINALLY,
    THROW,
    NEW,

    // Literals
    NUMBER,
    STRING,
    BOOLEAN,
    NULL_TOKEN,
    UNDEFINED_TOKEN,

    // Identifiers
    IDENTIFIER,

    // Operators
    PLUS,
    MINUS,
    STAR,
    SLASH,
    PERCENT,
    POWER,          // **

    // Update operators
    PLUS_PLUS,      // ++
    MINUS_MINUS,    // --

    // Compound assignment
    PLUS_EQUAL,     // +=
    MINUS_EQUAL,    // -=

    // Equal operators
    EQUAL,
    EQUAL_EQUAL,
    STRICT_EQUAL,

    NOT_EQUAL,
    NOT_STRICT_EQUAL,  // !==

    LESS,
    LESS_EQUAL,

    GREATER,
    GREATER_EQUAL,

    // Logical
    AND,
    OR,
    NOT,

    // Arrow
    ARROW,          // =>

    // Spread
    SPREAD,         // ...

    // Delimiters
    LPAREN,
    RPAREN,

    LBRACE,
    RBRACE,

    LBRACKET,
    RBRACKET,

    COMMA,
    COLON,
    DOT,
    SEMICOLON,

    // End
    END_OF_FILE
};

struct Token
{
    TokenType type;
    std::string value;

    Token(TokenType type, const std::string& value) : type(type), value(value)
    {}
};

inline std::string tokenTypeToString(TokenType type)
{
    switch(type)
    {
        case TokenType::LET:
            return "LET";

        case TokenType::CONST:
            return "CONST";

        case TokenType::DO:
            return "DO";
            
        case TokenType::IDENTIFIER:
            return "IDENTIFIER";
            
        case TokenType::BOOLEAN:
            return "BOOLEAN";

        case TokenType::NULL_TOKEN:
            return "NULL";

        case TokenType::UNDEFINED_TOKEN:
            return "UNDEFINED";

        case TokenType::NUMBER:
            return "NUMBER";

        case TokenType::STRING:
            return "STRING";

        case TokenType::PLUS:
            return "PLUS";

        case TokenType::MINUS:
            return "MINUS";

        case TokenType::STAR:
            return "STAR";

        case TokenType::SLASH:
            return "SLASH";

        case TokenType::PERCENT:
            return "PERCENT";

        case TokenType::POWER:
            return "POWER";

        case TokenType::PLUS_PLUS:
            return "PLUS_PLUS";

        case TokenType::MINUS_MINUS:
            return "MINUS_MINUS";

        case TokenType::PLUS_EQUAL:
            return "PLUS_EQUAL";

        case TokenType::MINUS_EQUAL:
            return "MINUS_EQUAL";

        case TokenType::AND:
            return "AND";

        case TokenType::OR:
            return "OR";

        case TokenType::EQUAL:
            return "EQUAL";

        case TokenType::EQUAL_EQUAL:
            return "LOOSE_EQUAL";

        case TokenType::STRICT_EQUAL:
            return "STRICT_EQUAL";

        case TokenType::GREATER:
            return "GREATER";
        
        case TokenType::GREATER_EQUAL:
            return "GREATER_EQUAL";

        case TokenType::LESS:
            return "LESS";

        case TokenType::LESS_EQUAL:
            return "LESS_EQUAL";

        case TokenType::NOT:
            return "NOT";
        
        case TokenType::NOT_EQUAL:
            return "NOT_EQUAL";

        case TokenType::NOT_STRICT_EQUAL:
            return "NOT_STRICT_EQUAL";

        case TokenType::ARROW:
            return "ARROW";

        case TokenType::SPREAD:
            return "SPREAD";
        
        case TokenType::COMMA:
            return "COMMA";

        case TokenType::COLON:
            return "COLON";

        case TokenType::DOT:
            return "DOT";

        case TokenType::SEMICOLON:
            return "SEMICOLON";

        case TokenType::LPAREN:
            return "LPAREN";

        case TokenType::RPAREN:
            return "RPAREN";

        case TokenType::LBRACE: 
            return "LBRACE";

        case TokenType::RBRACE:
            return "RBRACE";

        case TokenType::LBRACKET:
            return "LBRACKET";
        
        case TokenType::RBRACKET:
            return "RBRACKET";

        case TokenType::END_OF_FILE:
            return "EOF";

        case TokenType::IF:
            return "IF";

        case TokenType::ELSE:
            return "ELSE";

        case TokenType::WHILE:
            return "WHILE";

        case TokenType::FOR:
            return "FOR";

        case TokenType::SWITCH:
            return "SWITCH";

        case TokenType::CASE:
            return "CASE";

        case TokenType::DEFAULT:
            return "DEFAULT";

        case TokenType::BREAK:
            return "BREAK";

        case TokenType::RETURN:
            return "RETURN";

        case TokenType::FUNCTION:
            return "FUNCTION";

        case TokenType::TRY:
            return "TRY";

        case TokenType::CATCH:
            return "CATCH";

        case TokenType::FINALLY:
            return "FINALLY";

        case TokenType::THROW:
            return "THROW";

        case TokenType::NEW:
            return "NEW";

        default:
            return "UNKNOWN";
    }
}
