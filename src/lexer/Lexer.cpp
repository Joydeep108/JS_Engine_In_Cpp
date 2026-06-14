#include "Lexer.hpp"

Lexer::Lexer(const std::string &source) : source(source), current(0)
{
}

bool Lexer::isAtEnd() const
{
    return current >= source.length();
}

char Lexer::lookahead() const
{
    if (current + 1 >= source.length())
        return '\0';

    return source[current + 1];
}

char Lexer::peek() const
{
    if (isAtEnd())
        return '\0';

    return source[current];
}

char Lexer::advance()
{
    return source[current++];
}

void Lexer::skipWhitespace()
{
    while (!isAtEnd())
    {
        char c = peek();

        if (
            c == ' ' ||
            c == '\t' ||
            c == '\r' ||
            c == '\n')
        {
            advance();
        }
        else
        {
            break;
        }
    }
}

Token Lexer::number()
{
    std::string value;

    while (
        !isAtEnd() &&
        (isdigit(peek()) || peek() == '.'))
    {
        value += advance();
    }

    return Token(
        TokenType::NUMBER,
        value);
}

Token Lexer::identifier()
{
    std::string value;

    while (
        !isAtEnd() &&
        (isalnum(peek()) ||
         peek() == '_'))
    {
        value += advance();
    }

    if (value == "let")
        return Token(TokenType::LET, value);

    if (value == "const")
        return Token(TokenType::CONST, value);

    if (value == "function")
        return Token(TokenType::FUNCTION, value);

    if (value == "if")
        return Token(TokenType::IF, value);

    if (value == "else")
        return Token(TokenType::ELSE, value);

    if (value == "do")
        return Token(TokenType::DO, value);

    if (value == "for")
        return Token(TokenType::FOR, value);

    if (value == "while")
        return Token(TokenType::WHILE, value);

    if (value == "switch")
        return Token(TokenType::SWITCH, value);

    if (value == "case")
        return Token(TokenType::CASE, value);

    if (value == "default")
        return Token(TokenType::DEFAULT, value);

    if (value == "break")
        return Token(TokenType::BREAK, value);

    if (value == "return")
        return Token(TokenType::RETURN, value);

    if (value == "try")
        return Token(TokenType::TRY, value);

    if (value == "catch")
        return Token(TokenType::CATCH, value);

    if (value == "finally")
        return Token(TokenType::FINALLY, value);

    if (value == "throw")
        return Token(TokenType::THROW, value);

    if (value == "new")
        return Token(TokenType::NEW, value);

    if (value == "true" || value == "false")
        return Token(TokenType::BOOLEAN, value);

    if (value == "null")
        return Token(TokenType::NULL_TOKEN, value);

    if (value == "undefined")
        return Token(TokenType::UNDEFINED_TOKEN, value);

    return Token(
        TokenType::IDENTIFIER,
        value);
}

Token Lexer::string()
{
    char quote = advance();

    std::string value;

    while (
        !isAtEnd() &&
        peek() != quote)
    {
        if (peek() == '\\')
        {
            advance();
            if (!isAtEnd())
            {
                char c = advance();
                switch (c)
                {
                case 'n':
                    value += '\n';
                    break;
                case 't':
                    value += '\t';
                    break;
                case '\\':
                    value += '\\';
                    break;
                case '"':
                    value += '"';
                    break;
                case '\'':
                    value += '\'';
                    break;
                default:
                    value += c;
                    break;
                }
            }
        }
        else
        {
            value += advance();
        }
    }

    if (!isAtEnd())
        advance();

    return Token(
        TokenType::STRING,
        value);
}

std::vector<Token> Lexer::tokenize()
{
    std::vector<Token> tokens;

    while (!isAtEnd())
    {
        skipWhitespace();

        if (isAtEnd())
            break;

        char c = peek();

        if (isdigit(c))
        {
            tokens.push_back(number());
            continue;
        }

        if (
            isalpha(c) ||
            c == '_')
        {
            tokens.push_back(identifier());
            continue;
        }

        switch (c)
        {
        case '+':
        {
            advance();

            if (peek() == '+')
            {
                advance();
                tokens.emplace_back(
                    TokenType::PLUS_PLUS,
                    "++");
            }
            else if (peek() == '=')
            {
                advance();
                tokens.emplace_back(
                    TokenType::PLUS_EQUAL,
                    "+=");
            }
            else
            {
                tokens.emplace_back(
                    TokenType::PLUS,
                    "+");
            }
            break;
        }

        case '-':
        {
            advance();

            if (peek() == '-')
            {
                advance();
                tokens.emplace_back(
                    TokenType::MINUS_MINUS,
                    "--");
            }
            else if (peek() == '=')
            {
                advance();
                tokens.emplace_back(
                    TokenType::MINUS_EQUAL,
                    "-=");
            }
            else
            {
                tokens.emplace_back(
                    TokenType::MINUS,
                    "-");
            }
            break;
        }

        case '*':
        {
            advance();

            if (peek() == '*')
            {
                advance();
                tokens.emplace_back(
                    TokenType::POWER,
                    "**");
            }
            else
            {
                tokens.emplace_back(
                    TokenType::STAR,
                    "*");
            }
            break;
        }

        case '/':
        {
            advance();

            if (peek() == '/')
            {
                advance();

                skipSingleLineComment();

                break;
            }

            if (peek() == '*')
            {
                advance();

                skipMultiLineComment();

                break;
            }

            tokens.emplace_back(
                TokenType::SLASH,
                "/");

            break;
        }

        case '%':
        {
            advance();

            tokens.emplace_back(
                TokenType::PERCENT,
                "%");

            break;
        }

        case '=':
        {
            advance();

            if (peek() == '=')
            {
                advance();

                if (peek() == '=')
                {
                    advance();

                    tokens.emplace_back(
                        TokenType::STRICT_EQUAL,
                        "===");
                }
                else
                {
                    tokens.emplace_back(
                        TokenType::EQUAL_EQUAL,
                        "==");
                }
            }
            else if (peek() == '>')
            {
                advance();

                tokens.emplace_back(
                    TokenType::ARROW,
                    "=>");
            }
            else
            {
                tokens.emplace_back(
                    TokenType::EQUAL,
                    "=");
            }

            break;
        }

        case '!':
        {
            advance();

            if (peek() == '=')
            {
                advance();

                if (peek() == '=')
                {
                    advance();

                    tokens.emplace_back(
                        TokenType::NOT_STRICT_EQUAL,
                        "!==");
                }
                else
                {
                    tokens.emplace_back(
                        TokenType::NOT_EQUAL,
                        "!=");
                }
            }
            else
            {
                tokens.emplace_back(
                    TokenType::NOT,
                    "!");
            }

            break;
        }

        case '>':
        {
            advance();

            if(peek() == '=')
            {
                advance();

                tokens.emplace_back(
                    TokenType::GREATER_EQUAL,
                    ">="
                );
            }
            else
            {
                tokens.emplace_back(
                    TokenType::GREATER,
                    ">"
                );
            }

            break;
        }

        case '<':
        {
            advance();

            if (peek() == '=')
            {
                advance();

                tokens.emplace_back(
                    TokenType::LESS_EQUAL,
                    "<=");
            }
            else
            {
                tokens.emplace_back(
                    TokenType::LESS,
                    "<");
            }

            break;
        }

        case '&':
        {
            advance();

            if (peek() == '&')
            {
                advance();

                tokens.emplace_back(
                    TokenType::AND,
                    "&&");
            }

            break;
        }

        case '.':
        {
            advance();

            if (peek() == '.' && lookahead() == '.')
            {
                advance();
                advance();

                tokens.emplace_back(
                    TokenType::SPREAD,
                    "...");
            }
            else
            {
                tokens.emplace_back(
                    TokenType::DOT,
                    ".");
            }

            break;
        }

        case '[':
        {
            advance();

            tokens.emplace_back(
                TokenType::LBRACKET,
                "[");

            break;
        }

        case ']':
        {
            advance();

            tokens.emplace_back(
                TokenType::RBRACKET,
                "]");

            break;
        }

        case ',':
        {
            advance();

            tokens.emplace_back(
                TokenType::COMMA,
                ",");

            break;
        }

        case '|':
        {
            advance();

            if(peek() == '|')
            {
                advance();

                tokens.emplace_back(
                    TokenType::OR,
                    "||");
            }

            break;
        }

        case ':':
        {
            advance();

            tokens.emplace_back(
                TokenType::COLON,
                ":");

            break;
        }

        case '(':
            advance();
            tokens.emplace_back(
                TokenType::LPAREN,
                "(");
            break;

        case ')':
            advance();
            tokens.emplace_back(
                TokenType::RPAREN,
                ")");
            break;

        case '{':
            advance();
            tokens.emplace_back(
                TokenType::LBRACE,
                "{");
            break;

        case '}':
            advance();
            tokens.emplace_back(
                TokenType::RBRACE,
                "}");
            break;

        case ';':
            advance();
            tokens.emplace_back(
                TokenType::SEMICOLON,
                ";");
            break;

        case '"':
        case '\'':
            tokens.push_back(string());
            break;

        default:
            advance();
            break;
        }
    }

    tokens.emplace_back(TokenType::END_OF_FILE, "EOF");

    return tokens;
}

// helper function
void Lexer::skipSingleLineComment()
{
    while (
        !isAtEnd() &&
        peek() != '\n')
    {
        advance();
    }
}

void Lexer::skipMultiLineComment()
{
    while (!isAtEnd())
    {
        if (
            peek() == '*' &&
            lookahead() == '/')
        {
            advance();
            advance();

            break;
        }

        advance();
    }
}
