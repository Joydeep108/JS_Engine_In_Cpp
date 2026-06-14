#pragma once

#include <vector>
#include <memory>

#include "Statements.hpp"


class Program : public ASTNode
{
public:

    std::vector<std::unique_ptr<Statement>> statements;
};