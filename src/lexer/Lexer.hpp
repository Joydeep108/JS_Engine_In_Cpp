#pragma once

#include <vector>
#include <string>

#include "Token.hpp"

class Lexer
{
private:

    std::string source;
    size_t current;

public:

    Lexer(const std::string& source);

    std::vector<Token> tokenize();

private:

    char lookahead() const;

    char peek() const;

    char advance();

    bool isAtEnd() const;

    void skipWhitespace();

    Token number();

    Token identifier();

    Token string();

    void skipSingleLineComment();

    void skipMultiLineComment();


};