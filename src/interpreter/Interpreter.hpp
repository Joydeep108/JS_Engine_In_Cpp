#pragma once

#include <vector>

#include "../ast/Expressions.hpp"
#include "../ast/Program.hpp"
#include "../ast/Statements.hpp"
#include "../runtime/Environment.hpp"

class Interpreter
{
private:
    Environment global;
    Environment* currentEnv;

public:
    Interpreter();

    void execute(
        Program* program
    );

    Value evaluate(
        Expression* expr
    );

private:
    void executeStatement(
        Statement* stmt
    );

    void executeStatements(
        const std::vector<
            std::unique_ptr<Statement>
        >& statements
    );

    void executeBlock(
        BlockStatement* block
    );

    Value callFunction(
        const std::shared_ptr<FunctionValue>& function,
        const std::vector<Value>& arguments
    );
};
