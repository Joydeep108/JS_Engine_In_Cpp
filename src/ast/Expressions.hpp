#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "ASTNode.hpp"

class BlockStatement;

class Expression : public ASTNode
{
public:
    virtual ~Expression() = default;
};

class NumberLiteral : public Expression
{
public:

    double value;

    NumberLiteral(double value): value(value)
    {}
};

class StringLiteral : public Expression
{
public:

    std::string value;

    StringLiteral(const std::string& value): value(value)
    {}
};

class NullLiteral : public Expression
{
};

class UndefinedLiteral : public Expression
{
};

class Identifier : public Expression
{
public:

    std::string name;

    Identifier(const std::string& name): name(name)
    {}
};

class BinaryExpression : public Expression
{
public:

    std::unique_ptr<Expression> left;

    std::string op;

    std::unique_ptr<Expression> right;

    BinaryExpression(
        std::unique_ptr<Expression> left,
        const std::string& op,
        std::unique_ptr<Expression> right
    )
        : left(std::move(left)),
          op(op),
          right(std::move(right))
    {}
};  

class UnaryExpression : public Expression
{
public:

    std::string op;

    std::unique_ptr<Expression> argument;

    UnaryExpression(
        const std::string& op,
        std::unique_ptr<Expression> argument
    )
        : op(op),
          argument(std::move(argument))
    {
    }
};

class UpdateExpression : public Expression
{
public:

    std::string op;

    std::unique_ptr<Expression> argument;

    bool prefix;

    UpdateExpression(
        const std::string& op,
        std::unique_ptr<Expression> argument,
        bool prefix
    )
        : op(op),
          argument(std::move(argument)),
          prefix(prefix)
    {
    }
};

class CompoundAssignmentExpression : public Expression
{
public:

    std::unique_ptr<Expression> left;

    std::string op;

    std::unique_ptr<Expression> right;

    CompoundAssignmentExpression(
        std::unique_ptr<Expression> left,
        const std::string& op,
        std::unique_ptr<Expression> right
    )
        : left(std::move(left)),
          op(op),
          right(std::move(right))
    {
    }
};

class AssignmentExpression : public Expression
{
public:

    std::unique_ptr<Expression> left;

    std::unique_ptr<Expression> value;

    AssignmentExpression(
        std::unique_ptr<Expression> left,
        std::unique_ptr<Expression> value
    )
        : left(std::move(left)),
          value(std::move(value))
    {
    }
};

class MemberExpression : public Expression
{
public:

    std::unique_ptr<Expression> object;

    std::string property;

    MemberExpression(
        std::unique_ptr<Expression> object,
        const std::string& property
    )
        : object(std::move(object)),
          property(property)
    {
    }
};

class IndexExpression : public Expression
{
public:

    std::unique_ptr<Expression> object;

    std::unique_ptr<Expression> index;

    IndexExpression(
        std::unique_ptr<Expression> object,
        std::unique_ptr<Expression> index
    )
        : object(std::move(object)),
          index(std::move(index))
    {
    }
};

class CallExpression : public Expression
{
public:

    std::unique_ptr<Expression> callee;

    std::vector<
        std::unique_ptr<Expression>
    > arguments;

    CallExpression(
        std::unique_ptr<Expression> callee
    )
        : callee(std::move(callee))
    {
    }
};

class ArrayLiteral : public Expression
{
public:

    std::vector<
        std::unique_ptr<Expression>
    > elements;
};

class SpreadElement : public Expression
{
public:

    std::unique_ptr<Expression> argument;

    SpreadElement(
        std::unique_ptr<Expression> argument
    )
        : argument(std::move(argument))
    {
    }
};

class Property
{
public:

    std::string key;

    std::unique_ptr<Expression> value;

    Property(
        std::string key,
        std::unique_ptr<Expression> value
    )
        : key(std::move(key)),
          value(std::move(value))
    {
    }
};

class ObjectLiteral : public Expression
{
public:

    std::vector<Property> properties;
};

class BooleanLiteral
    : public Expression
{
public:

    bool value;

    BooleanLiteral(
        bool value
    )
        : value(value)
    {
    }
};

class FunctionExpression : public Expression
{
public:

    std::string name;

    std::vector<std::string> parameters;

    std::unique_ptr<BlockStatement> body;

    FunctionExpression(
        const std::string& name,
        std::vector<std::string> parameters,
        std::unique_ptr<BlockStatement> body
    )
        : name(name),
          parameters(std::move(parameters)),
          body(std::move(body))
    {
    }
};

class ArrowFunctionExpression : public Expression
{
public:

    std::vector<std::string> parameters;

    std::unique_ptr<BlockStatement> body;

    std::unique_ptr<Expression> expression;

    ArrowFunctionExpression(
        std::vector<std::string> parameters,
        std::unique_ptr<BlockStatement> body,
        std::unique_ptr<Expression> expression
    )
        : parameters(std::move(parameters)),
          body(std::move(body)),
          expression(std::move(expression))
    {
    }
};
