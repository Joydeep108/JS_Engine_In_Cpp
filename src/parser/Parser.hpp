#pragma once

#include <vector>
#include <memory>

#include "../lexer/Token.hpp"

#include "../ast/Program.hpp"
#include "../ast/Statements.hpp"
#include "../ast/Expressions.hpp"

class Parser
{
private:

    std::vector<Token> tokens;
    size_t current;

public:

    Parser(const std::vector<Token>& tokens);

    std::unique_ptr<Program> parseProgram();

private:

    Token peek() const;
    Token advance();
    bool isAtEnd() const;
    bool check(TokenType type) const;
    bool match(TokenType type);

    std::unique_ptr<Statement> parseStatement();

    std::unique_ptr<VariableDeclaration>
    parseVariableDeclaration();

    std::unique_ptr<Expression>
    parseExpression();

    std::unique_ptr<Expression>
    parseLogicalOr();

    std::unique_ptr<Expression>
    parseLogicalAnd();

    std::unique_ptr<Expression>
    parseEquality();

    std::unique_ptr<Expression>
    parseComparison();

    std::unique_ptr<Expression>
    parseTerm();

    std::unique_ptr<Expression>
    parseFactor();

    std::unique_ptr<Expression>
    parseExponentiation();

    std::unique_ptr<Expression>
    parseUnary();

    std::unique_ptr<Expression>
    parsePrimary();

    std::unique_ptr<Expression>
    parseAssignment();

    std::unique_ptr<Expression>
    parseCall();

    std::unique_ptr<Expression>
    parseMember();

    std::unique_ptr<BlockStatement>
    parseBlockStatement();

    std::unique_ptr<IfStatement>
    parseIfStatement();

    std::unique_ptr<WhileStatement>
    parseWhileStatement();

    std::unique_ptr<DoWhileStatement>
    parseDoWhileStatement();

    std::unique_ptr<ForStatement>
    parseForStatement();

    std::unique_ptr<SwitchStatement>
    parseSwitchStatement();

    std::unique_ptr<BreakStatement>
    parseBreakStatement();

    std::unique_ptr<FunctionDeclaration>
    parseFunctionDeclaration();

    std::unique_ptr<ReturnStatement>
    parseReturnStatement();

    std::unique_ptr<TryCatchStatement>
    parseTryCatchStatement();

    std::unique_ptr<ThrowStatement>
    parseThrowStatement();

};
