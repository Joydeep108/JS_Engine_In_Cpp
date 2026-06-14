#include "Parser.hpp"
#include <stdexcept>

Parser::Parser(const std::vector<Token>& tokens): tokens(tokens), current(0)
{
}

// Helper function
Token Parser::peek() const
{
    return tokens[current];
}

Token Parser::advance()
{
    return tokens[current++];
}

bool Parser::isAtEnd() const
{
    return peek().type == TokenType::END_OF_FILE;
}

bool Parser::check(TokenType type) const
{
    if(isAtEnd())
        return false;

    return peek().type == type;
}

bool Parser::match(TokenType type)
{
    if(check(type))
    {
        advance();
        return true;
    }

    return false;
}

std::unique_ptr<Program>
Parser::parseProgram()
{   
    auto program =
        std::make_unique<Program>();

    while(!isAtEnd())
    {
        auto stmt =
            parseStatement();

        if(stmt)
        {
            program->statements.push_back(
                std::move(stmt)
            );
        }
    }

    return program;
}

std::unique_ptr<Statement>
Parser::parseStatement()
{
    if(check(TokenType::LET) ||
       check(TokenType::CONST))
    {
        return parseVariableDeclaration();
    }

    if(check(TokenType::IF))
    {
        return parseIfStatement();
    }

    if(check(TokenType::WHILE))
    {
        return parseWhileStatement();
    }

    if(check(TokenType::DO))
    {
        return parseDoWhileStatement();
    }

    if(check(TokenType::FOR))
    {
        return parseForStatement();
    }

    if(check(TokenType::SWITCH))
    {
        return parseSwitchStatement();
    }

    if(check(TokenType::BREAK))
    {
        return parseBreakStatement();
    }

    if(check(TokenType::FUNCTION))
    {
        return parseFunctionDeclaration();
    }

    if(check(TokenType::RETURN))
    {
        return parseReturnStatement();
    }

    if(check(TokenType::TRY))
    {
        return parseTryCatchStatement();
    }

    if(check(TokenType::THROW))
    {
        return parseThrowStatement();
    }

    auto expr = parseExpression();

    match(TokenType::SEMICOLON);

    return std::make_unique<
        ExpressionStatement
    >(
        std::move(expr)
    );
}

std::unique_ptr<VariableDeclaration>
Parser::parseVariableDeclaration()
{
    advance(); // consume LET or CONST

    Token identifier =
        advance();

    std::unique_ptr<Expression>
    initializer = nullptr;

    if(match(TokenType::EQUAL))
    {
        initializer =
            parseExpression();
    }

    match(TokenType::SEMICOLON);

    return std::make_unique<
        VariableDeclaration
    >(
        identifier.value,
        std::move(initializer)
    );
}

std::unique_ptr<Expression>
Parser::parsePrimary()
{
    // Parenthesized expression or arrow function
    if(check(TokenType::LPAREN))
    {
        size_t saved = current;
        advance(); // consume (

        // Try to parse as arrow function parameters
        bool isArrow = true;
        std::vector<std::string> params;

        if(!check(TokenType::RPAREN))
        {
            do
            {
                if(!check(TokenType::IDENTIFIER))
                {
                    isArrow = false;
                    break;
                }
                params.push_back(advance().value);
            }
            while(match(TokenType::COMMA));
        }

        if(isArrow &&
           match(TokenType::RPAREN) &&
           check(TokenType::ARROW))
        {
            advance(); // consume =>

            if(check(TokenType::LBRACE))
            {
                auto body = parseBlockStatement();
                return std::make_unique<
                    ArrowFunctionExpression
                >(
                    std::move(params),
                    std::move(body),
                    nullptr
                );
            }
            else
            {
                auto expr = parseAssignment();
                return std::make_unique<
                    ArrowFunctionExpression
                >(
                    std::move(params),
                    nullptr,
                    std::move(expr)
                );
            }
        }

        // Not an arrow function, backtrack
        current = saved;
        advance(); // consume (

        auto expr = parseExpression();

        if(!match(TokenType::RPAREN))
        {
            throw std::runtime_error(
                "Expected ')'"
            );
        }

        return expr;
    }

    // Array literal
    if(match(TokenType::LBRACKET))
    {
        auto array =
            std::make_unique<
                ArrayLiteral
            >();

        if(!check(TokenType::RBRACKET))
        {
            do
            {
                if(match(TokenType::SPREAD))
                {
                    array->elements.push_back(
                        std::make_unique<SpreadElement>(
                            parseAssignment()
                        )
                    );
                }
                else
                {
                    array->elements.push_back(
                        parseAssignment()
                    );
                }
            }
            while(match(TokenType::COMMA));
        }

        if(!match(TokenType::RBRACKET))
        {
            throw std::runtime_error(
                "Expected ']'"
            );
        }

        return array;
    }

    // Object literal
    if(match(TokenType::LBRACE))
    {
        auto object =
            std::make_unique<
                ObjectLiteral
            >();

        if(!check(TokenType::RBRACE))
        {
            do
            {
                Token key =
                    advance();

                if(
                    key.type != TokenType::IDENTIFIER &&
                    key.type != TokenType::STRING
                )
                {
                    throw std::runtime_error(
                        "Expected object property name"
                    );
                }

                if(!match(TokenType::COLON))
                {
                    throw std::runtime_error(
                        "Expected ':'"
                    );
                }

                object->properties.emplace_back(
                    key.value,
                    parseAssignment()
                );
            }
            while(match(TokenType::COMMA));
        }

        if(!match(TokenType::RBRACE))
        {
            throw std::runtime_error(
                "Expected '}'"
            );
        }

        return object;
    }

    // Function expression
    if(check(TokenType::FUNCTION))
    {
        advance(); // consume function

        std::string name;
        if(check(TokenType::IDENTIFIER))
        {
            name = advance().value;
        }

        match(TokenType::LPAREN);

        std::vector<std::string> parameters;

        if(!check(TokenType::RPAREN))
        {
            do
            {
                Token param = advance();
                parameters.push_back(param.value);
            }
            while(match(TokenType::COMMA));
        }

        match(TokenType::RPAREN);

        auto body = parseBlockStatement();

        return std::make_unique<
            FunctionExpression
        >(
            name,
            std::move(parameters),
            std::move(body)
        );
    }

    Token token = advance();

    switch(token.type)
    {
        case TokenType::NUMBER:
        {
            return std::make_unique<
                NumberLiteral
            >(
                std::stod(token.value)
            );
        }

        case TokenType::STRING:
        {
            return std::make_unique<
                StringLiteral
            >(
                token.value
            );
        }

        case TokenType::NULL_TOKEN:
        {
            return std::make_unique<
                NullLiteral
            >();
        }

        case TokenType::UNDEFINED_TOKEN:
        {
            return std::make_unique<
                UndefinedLiteral
            >();
        }

        case TokenType::IDENTIFIER:
        {
            // Check for single-param arrow function: x => expr
            if(check(TokenType::ARROW))
            {
                advance(); // consume =>
                std::vector<std::string> params = {token.value};

                if(check(TokenType::LBRACE))
                {
                    auto body = parseBlockStatement();
                    return std::make_unique<
                        ArrowFunctionExpression
                    >(
                        std::move(params),
                        std::move(body),
                        nullptr
                    );
                }
                else
                {
                    auto expr = parseAssignment();
                    return std::make_unique<
                        ArrowFunctionExpression
                    >(
                        std::move(params),
                        nullptr,
                        std::move(expr)
                    );
                }
            }

            return std::make_unique<
                Identifier
            >(
                token.value
            );
        }

        case TokenType::BOOLEAN:
        {
            return std::make_unique<
                BooleanLiteral
            >(
                token.value == "true"
            );
        }

        default: 
            throw std::runtime_error(
                    "Unexpected token: " +
                    tokenTypeToString(token.type) +
                    " '" + token.value + "'"
                );
    }
}

std::unique_ptr<Expression>
Parser::parseExpression()
{
    return parseAssignment();
}

std::unique_ptr<Expression>
Parser::parseExponentiation()
{
    auto left = parseUnary();

    if(check(TokenType::POWER))
    {
        Token op = advance();

        auto right =
            parseExponentiation(); // right-associative

        return std::make_unique<
            BinaryExpression
        >(
            std::move(left),
            op.value,
            std::move(right)
        );
    }

    return left;
}

std::unique_ptr<Expression>
Parser::parseFactor()
{
    auto left = parseExponentiation();

    while(
        check(TokenType::STAR) ||
        check(TokenType::SLASH) ||
        check(TokenType::PERCENT)
    )
    {
        Token op = advance();

        auto right =
            parseExponentiation();

        left =
            std::make_unique<
                BinaryExpression
            >(
                std::move(left),
                op.value,
                std::move(right)
            );
    }

    return left;
}

std::unique_ptr<Expression>
Parser::parseUnary()
{
    if(check(TokenType::NEW))
    {
        advance(); // consume NEW
        return parseUnary();
    }

    // Prefix ++ and --
    if(
        check(TokenType::PLUS_PLUS) ||
        check(TokenType::MINUS_MINUS)
    )
    {
        Token op = advance();
        auto argument = parseUnary();

        return std::make_unique<
            UpdateExpression
        >(
            op.value,
            std::move(argument),
            true
        );
    }

    if(
        check(TokenType::NOT) ||
        check(TokenType::MINUS)
    )
    {
        Token op = advance();

        return std::make_unique<
            UnaryExpression
        >(
            op.value,
            parseUnary()
        );
    }

    return parseCall();
}

std::unique_ptr<Expression>
Parser::parseTerm()
{
    auto left = parseFactor();

    while(
        check(TokenType::PLUS) ||
        check(TokenType::MINUS)
    )
    {
        Token op = advance();

        auto right =
            parseFactor();

        left =
            std::make_unique<
                BinaryExpression
            >(
                std::move(left),
                op.value,
                std::move(right)
            );
    }

    return left;
}

std::unique_ptr<Expression>
Parser::parseComparison()
{
    auto left = parseTerm();

    while(
        check(TokenType::LESS) ||
        check(TokenType::LESS_EQUAL) ||
        check(TokenType::GREATER) ||
        check(TokenType::GREATER_EQUAL)
    )
    {
        Token op = advance();

        auto right =
            parseTerm();

        left =
            std::make_unique<
                BinaryExpression
            >(
                std::move(left),
                op.value,
                std::move(right)
            );
    }

    return left;
}

std::unique_ptr<Expression>
Parser::parseLogicalAnd()
{
    auto left =
        parseEquality();

    while(check(TokenType::AND))
    {
        Token op = advance();

        auto right =
            parseEquality();

        left =
            std::make_unique<
                BinaryExpression
            >(
                std::move(left),
                op.value,
                std::move(right)
            );
    }

    return left;
}

std::unique_ptr<Expression>
Parser::parseLogicalOr()
{
    auto left =
        parseLogicalAnd();

    while(check(TokenType::OR))
    {
        Token op = advance();

        auto right =
            parseLogicalAnd();

        left =
            std::make_unique<
                BinaryExpression
            >(
                std::move(left),
                op.value,
                std::move(right)
            );
    }

    return left;
}

std::unique_ptr<Expression>
Parser::parseEquality()
{
    auto left =
        parseComparison();

    while(
        check(TokenType::EQUAL_EQUAL) ||
        check(TokenType::STRICT_EQUAL) ||
        check(TokenType::NOT_EQUAL) ||
        check(TokenType::NOT_STRICT_EQUAL)
    )
    {
        Token op = advance();

        auto right =
            parseComparison();

        left =
            std::make_unique<
                BinaryExpression
            >(
                std::move(left),
                op.value,
                std::move(right)
            );
    }

    return left;
}

std::unique_ptr<Expression>
Parser::parseAssignment()
{
    auto left =
        parseLogicalOr();

    if(match(TokenType::EQUAL))
    {
        auto value =
            parseAssignment();

        return std::make_unique<
            AssignmentExpression
        >(
            std::move(left),
            std::move(value)
        );
    }

    // Compound assignment: +=, -=
    if(check(TokenType::PLUS_EQUAL) ||
       check(TokenType::MINUS_EQUAL))
    {
        Token op = advance();

        auto right =
            parseAssignment();

        return std::make_unique<
            CompoundAssignmentExpression
        >(
            std::move(left),
            op.value,
            std::move(right)
        );
    }

    return left;
}

std::unique_ptr<Expression>
Parser::parseMember()
{
    auto object =
        parsePrimary();

    while(true)
    {
        if(match(TokenType::DOT))
        {
            Token property =
                advance();

            object =
                std::make_unique<
                    MemberExpression
                >(
                    std::move(object),
                    property.value
                );

            continue;
        }

        if(match(TokenType::LBRACKET))
        {
            auto index =
                parseExpression();

            if(!match(TokenType::RBRACKET))
            {
                throw std::runtime_error(
                    "Expected ']'"
                );
            }

            object =
                std::make_unique<
                    IndexExpression
                >(
                    std::move(object),
                    std::move(index)
                );

            continue;
        }

        break;
    }

    return object;
}

std::unique_ptr<Expression>
Parser::parseCall()
{
    auto callee =
        parseMember();

    while(match(TokenType::LPAREN))
    {
        auto call =
            std::make_unique<
                CallExpression
            >(
                std::move(callee)
            );

        if(!check(TokenType::RPAREN))
        {
            do
            {
                if(match(TokenType::SPREAD))
                {
                    call->arguments.push_back(
                        std::make_unique<SpreadElement>(
                            parseExpression()
                        )
                    );
                }
                else
                {
                    call->arguments.push_back(
                        parseExpression()
                    );
                }

            } while(
                match(TokenType::COMMA)
            );
        }

        match(TokenType::RPAREN);

        callee = std::move(call);

        // Allow chaining member access after call: str.split("").reverse()
        while(match(TokenType::DOT))
        {
            Token property = advance();

            callee = std::make_unique<
                MemberExpression
            >(
                std::move(callee),
                property.value
            );

            if(match(TokenType::LPAREN))
            {
                auto chainCall = std::make_unique<
                    CallExpression
                >(
                    std::move(callee)
                );

                if(!check(TokenType::RPAREN))
                {
                    do
                    {
                        if(match(TokenType::SPREAD))
                        {
                            chainCall->arguments.push_back(
                                std::make_unique<SpreadElement>(
                                    parseExpression()
                                )
                            );
                        }
                        else
                        {
                            chainCall->arguments.push_back(
                                parseExpression()
                            );
                        }
                    }
                    while(match(TokenType::COMMA));
                }

                match(TokenType::RPAREN);
                callee = std::move(chainCall);
            }
            else if(match(TokenType::LBRACKET))
            {
                auto index = parseExpression();
                if(!match(TokenType::RBRACKET))
                {
                    throw std::runtime_error("Expected ']'");
                }
                callee = std::make_unique<IndexExpression>(
                    std::move(callee),
                    std::move(index)
                );
            }
        }
    }

    // Postfix ++ and --
    if(check(TokenType::PLUS_PLUS) ||
       check(TokenType::MINUS_MINUS))
    {
        Token op = advance();
        return std::make_unique<
            UpdateExpression
        >(
            op.value,
            std::move(callee),
            false
        );
    }

    return callee;
}

std::unique_ptr<BlockStatement>
Parser::parseBlockStatement()
{
    auto block =
        std::make_unique<
            BlockStatement
        >();

    if(!match(TokenType::LBRACE))
    {
        throw std::runtime_error(
            "Expected '{'"
        );
    }

    while(
        !check(TokenType::RBRACE)
        &&
        !isAtEnd()
    )
    {
        block->statements.push_back(
            parseStatement()
        );
    }

    if(!match(TokenType::RBRACE))
    {
        throw std::runtime_error(
            "Expected '}'"
        );
    }

    return block;
}

std::unique_ptr<IfStatement>
Parser::parseIfStatement()
{
    advance(); // consume IF

    if(!match(TokenType::LPAREN))
    {
        throw std::runtime_error(
            "Expected '('"
        );
    }

    auto condition =
        parseExpression();

    if(!match(TokenType::RPAREN))
    {
        throw std::runtime_error(
            "Expected ')'"
        );
    }

    std::unique_ptr<BlockStatement> thenBlock;
    if(check(TokenType::LBRACE))
    {
        thenBlock = parseBlockStatement();
    }
    else
    {
        thenBlock = std::make_unique<BlockStatement>();
        thenBlock->statements.push_back(parseStatement());
    }

    std::unique_ptr<BlockStatement> elseBlock = nullptr;

    if(match(TokenType::ELSE))
    {
        if(check(TokenType::IF))
        {
            // else if: wrap in a block
            auto innerBlock =
                std::make_unique<BlockStatement>();
            innerBlock->statements.push_back(
                parseIfStatement()
            );
            elseBlock = std::move(innerBlock);
        }
        else if(check(TokenType::LBRACE))
        {
            elseBlock = parseBlockStatement();
        }
        else
        {
            elseBlock = std::make_unique<BlockStatement>();
            elseBlock->statements.push_back(parseStatement());
        }
    }

    return std::make_unique<
        IfStatement
    >(
        std::move(condition),
        std::move(thenBlock),
        std::move(elseBlock)
    );
}

std::unique_ptr<WhileStatement>
Parser::parseWhileStatement()
{
    advance(); // consume while

    if(!match(TokenType::LPAREN))
    {
        throw std::runtime_error(
            "Expected '('"
        );
    }

    auto condition =
        parseExpression();

    if(!match(TokenType::RPAREN))
    {
        throw std::runtime_error(
            "Expected ')'"
        );
    }

    std::unique_ptr<BlockStatement> body;
    if(check(TokenType::LBRACE))
    {
        body = parseBlockStatement();
    }
    else
    {
        body = std::make_unique<BlockStatement>();
        body->statements.push_back(parseStatement());
    }

    return std::make_unique<
        WhileStatement
    >(
        std::move(condition),
        std::move(body)
    );
}

std::unique_ptr<DoWhileStatement>
Parser::parseDoWhileStatement()
{
    advance(); // consume DO

    std::unique_ptr<BlockStatement> body;
    if(check(TokenType::LBRACE))
    {
        body = parseBlockStatement();
    }
    else
    {
        body = std::make_unique<BlockStatement>();
        body->statements.push_back(parseStatement());
    }

    if(!match(TokenType::WHILE))
    {
        throw std::runtime_error(
            "Expected 'while'"
        );
    }

    if(!match(TokenType::LPAREN))
    {
        throw std::runtime_error(
            "Expected '('"
        );
    }

    auto condition =
        parseExpression();

    if(!match(TokenType::RPAREN))
    {
        throw std::runtime_error(
            "Expected ')'"
        );
    }

    match(TokenType::SEMICOLON);

    return std::make_unique<
        DoWhileStatement
    >(
        std::move(body),
        std::move(condition)
    );
}

std::unique_ptr<ForStatement>
Parser::parseForStatement()
{
    advance(); // consume FOR

    if(!match(TokenType::LPAREN))
    {
        throw std::runtime_error(
            "Expected '('"
        );
    }

    // initializer
    std::unique_ptr<Statement>
    initializer = nullptr;

    if(check(TokenType::LET) ||
       check(TokenType::CONST))
    {
        initializer =
            parseVariableDeclaration();
    }
    else if(!check(TokenType::SEMICOLON))
    {
        auto expr = parseExpression();
        match(TokenType::SEMICOLON);
        initializer = std::make_unique<
            ExpressionStatement
        >(std::move(expr));
    }
    else
    {
        match(TokenType::SEMICOLON);
    }

    // condition
    std::unique_ptr<Expression> condition = nullptr;
    if(!check(TokenType::SEMICOLON))
    {
        condition = parseExpression();
    }

    if(!match(TokenType::SEMICOLON))
    {
        throw std::runtime_error(
            "Expected ';'"
        );
    }

    // update
    std::unique_ptr<Expression> update = nullptr;
    if(!check(TokenType::RPAREN))
    {
        update = parseExpression();
    }

    if(!match(TokenType::RPAREN))
    {
        throw std::runtime_error(
            "Expected ')'"
        );
    }

    // body
    std::unique_ptr<BlockStatement> body;
    if(check(TokenType::LBRACE))
    {
        body = parseBlockStatement();
    }
    else
    {
        body = std::make_unique<BlockStatement>();
        body->statements.push_back(parseStatement());
    }

    return std::make_unique<
        ForStatement
    >(
        std::move(initializer),
        std::move(condition),
        std::move(update),
        std::move(body)
    );
}

std::unique_ptr<SwitchStatement>
Parser::parseSwitchStatement()
{
    advance(); // consume SWITCH

    if(!match(TokenType::LPAREN))
    {
        throw std::runtime_error(
            "Expected '('"
        );
    }

    auto discriminant =
        parseExpression();

    if(!match(TokenType::RPAREN))
    {
        throw std::runtime_error(
            "Expected ')'"
        );
    }

    if(!match(TokenType::LBRACE))
    {
        throw std::runtime_error(
            "Expected '{'"
        );
    }

    auto switchStatement =
        std::make_unique<
            SwitchStatement
        >(
            std::move(discriminant)
        );

    while(
        !check(TokenType::RBRACE) &&
        !isAtEnd()
    )
    {
        std::unique_ptr<Expression>
        test = nullptr;

        if(match(TokenType::CASE))
        {
            test = parseExpression();
        }
        else if(!match(TokenType::DEFAULT))
        {
            throw std::runtime_error(
                "Expected 'case' or 'default'"
            );
        }

        if(!match(TokenType::COLON))
        {
            throw std::runtime_error(
                "Expected ':'"
            );
        }

        auto switchCase =
            std::make_unique<
                SwitchCase
            >(
                std::move(test)
            );

        while(
            !check(TokenType::CASE) &&
            !check(TokenType::DEFAULT) &&
            !check(TokenType::RBRACE) &&
            !isAtEnd()
        )
        {
            switchCase->statements.push_back(
                parseStatement()
            );
        }

        switchStatement->cases.push_back(
            std::move(switchCase)
        );
    }

    if(!match(TokenType::RBRACE))
    {
        throw std::runtime_error(
            "Expected '}'"
        );
    }

    return switchStatement;
}

std::unique_ptr<BreakStatement>
Parser::parseBreakStatement()
{
    advance(); // consume BREAK

    match(TokenType::SEMICOLON);

    return std::make_unique<
        BreakStatement
    >();
}

std::unique_ptr<ReturnStatement>
Parser::parseReturnStatement()
{
    advance(); // consume RETURN

    std::unique_ptr<Expression> value = nullptr;

    if(!check(TokenType::SEMICOLON) &&
       !check(TokenType::RBRACE) &&
       !isAtEnd())
    {
        value = parseExpression();
    }

    match(
        TokenType::SEMICOLON
    );

    return std::make_unique<
        ReturnStatement
    >(
        std::move(value)
    );
}


std::unique_ptr<FunctionDeclaration>
Parser::parseFunctionDeclaration()
{
    advance(); // function

    Token name =
        advance();

    match(TokenType::LPAREN);

    std::vector<std::string>
    parameters;

    if(!check(TokenType::RPAREN))
    {
        do
        {
            Token param =
                advance();

            parameters.push_back(
                param.value
            );

        } while(
            match(TokenType::COMMA)
        );
    }

    match(TokenType::RPAREN);

    auto body =
        parseBlockStatement();

    return std::make_unique<
        FunctionDeclaration
    >(
        name.value,

        std::move(parameters),

        std::move(body)
    );
}

std::unique_ptr<TryCatchStatement>
Parser::parseTryCatchStatement()
{
    advance(); // consume TRY

    auto tryBlock = parseBlockStatement();

    std::string catchParam;
    std::unique_ptr<BlockStatement> catchBlock = nullptr;

    if(match(TokenType::CATCH))
    {
        if(match(TokenType::LPAREN))
        {
            catchParam = advance().value;
            match(TokenType::RPAREN);
        }

        catchBlock = parseBlockStatement();
    }

    std::unique_ptr<BlockStatement> finallyBlock = nullptr;

    if(match(TokenType::FINALLY))
    {
        finallyBlock = parseBlockStatement();
    }

    return std::make_unique<
        TryCatchStatement
    >(
        std::move(tryBlock),
        catchParam,
        std::move(catchBlock),
        std::move(finallyBlock)
    );
}

std::unique_ptr<ThrowStatement>
Parser::parseThrowStatement()
{
    advance(); // consume THROW

    auto argument = parseExpression();

    match(TokenType::SEMICOLON);

    return std::make_unique<
        ThrowStatement
    >(
        std::move(argument)
    );
}
