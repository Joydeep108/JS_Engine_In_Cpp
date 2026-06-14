#pragma once

#include <memory>
#include <vector>

#include "ASTNode.hpp"
#include "Expressions.hpp"

class Statement : public ASTNode
{
public:
    virtual ~Statement() = default;
};

class VariableDeclaration : public Statement
{
public:

    std::string name;

    std::unique_ptr<Expression> initializer;

    VariableDeclaration(
        const std::string& name,
        std::unique_ptr<Expression> initializer
    )
        : name(name),
          initializer(std::move(initializer))
    {}
};

class ExpressionStatement : public Statement
{
public:

    std::unique_ptr<Expression> expression;

    ExpressionStatement(
        std::unique_ptr<Expression> expression
    )
        : expression(std::move(expression))
    {
    }
};

class BlockStatement : public Statement
{
public:

    std::vector<
        std::unique_ptr<Statement>
    > statements;
};

class IfStatement : public Statement
{
public:

    std::unique_ptr<Expression>
    condition;

    std::unique_ptr<BlockStatement>
    thenBlock;

    std::unique_ptr<BlockStatement>
    elseBlock;

    IfStatement(
        std::unique_ptr<Expression>
            condition,

        std::unique_ptr<BlockStatement>
            thenBlock,

        std::unique_ptr<BlockStatement>
            elseBlock
    )
        : condition(
            std::move(condition)
          ),

          thenBlock(
            std::move(thenBlock)
          ),

          elseBlock(
            std::move(elseBlock)
          )
    {
    }
};

class WhileStatement : public Statement
{
public:

    std::unique_ptr<Expression>
    condition;

    std::unique_ptr<BlockStatement>
    body;

    WhileStatement(
        std::unique_ptr<Expression>
            condition,

        std::unique_ptr<BlockStatement>
            body
    )
        : condition(
            std::move(condition)
          ),

          body(
            std::move(body)
          )
    {
    }
};

class DoWhileStatement : public Statement
{
public:

    std::unique_ptr<BlockStatement>
    body;

    std::unique_ptr<Expression>
    condition;

    DoWhileStatement(
        std::unique_ptr<BlockStatement>
            body,

        std::unique_ptr<Expression>
            condition
    )
        : body(
            std::move(body)
          ),
          condition(
            std::move(condition)
          )
    {
    }
};

class ForStatement : public Statement
{
public:

    std::unique_ptr<Statement>
    initializer;

    std::unique_ptr<Expression>
    condition;

    std::unique_ptr<Expression>
    update;

    std::unique_ptr<BlockStatement>
    body;

    ForStatement(
        std::unique_ptr<Statement>
            initializer,

        std::unique_ptr<Expression>
            condition,

        std::unique_ptr<Expression>
            update,

        std::unique_ptr<BlockStatement>
            body
    )
        : initializer(
            std::move(initializer)
          ),

          condition(
            std::move(condition)
          ),

          update(
            std::move(update)
          ),

          body(
            std::move(body)
          )
    {
    }
};

class BreakStatement : public Statement
{
};

class SwitchCase : public ASTNode
{
public:

    std::unique_ptr<Expression>
    test;

    std::vector<
        std::unique_ptr<Statement>
    > statements;

    SwitchCase(
        std::unique_ptr<Expression>
            test
    )
        : test(std::move(test))
    {
    }
};

class SwitchStatement : public Statement
{
public:

    std::unique_ptr<Expression>
    discriminant;

    std::vector<
        std::unique_ptr<SwitchCase>
    > cases;

    SwitchStatement(
        std::unique_ptr<Expression>
            discriminant
    )
        : discriminant(
            std::move(discriminant)
          )
    {
    }
};

class ReturnStatement : public Statement
{
public:

    std::unique_ptr<Expression>
    value;

    ReturnStatement(
        std::unique_ptr<Expression>
            value
    )
        : value(std::move(value))
    {
    }
};

class FunctionDeclaration : public Statement
{
public:

    std::string name;

    std::vector<std::string>
    parameters;

    std::unique_ptr<BlockStatement>
    body;

    FunctionDeclaration(
        const std::string& name,

        std::vector<std::string>
            parameters,

        std::unique_ptr<BlockStatement>
            body
    )
        : name(name),

          parameters(
            std::move(parameters)
          ),

          body(
            std::move(body)
          )
    {
    }
};

class ThrowStatement : public Statement
{
public:

    std::unique_ptr<Expression>
    argument;

    ThrowStatement(
        std::unique_ptr<Expression>
            argument
    )
        : argument(std::move(argument))
    {
    }
};

class TryCatchStatement : public Statement
{
public:

    std::unique_ptr<BlockStatement>
    tryBlock;

    std::string catchParam;

    std::unique_ptr<BlockStatement>
    catchBlock;

    std::unique_ptr<BlockStatement>
    finallyBlock;

    TryCatchStatement(
        std::unique_ptr<BlockStatement>
            tryBlock,

        const std::string& catchParam,

        std::unique_ptr<BlockStatement>
            catchBlock,

        std::unique_ptr<BlockStatement>
            finallyBlock
    )
        : tryBlock(
            std::move(tryBlock)
          ),
          catchParam(catchParam),
          catchBlock(
            std::move(catchBlock)
          ),
          finallyBlock(
            std::move(finallyBlock)
          )
    {
    }
};
