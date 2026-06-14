#pragma once

#include <unordered_map>
#include <string>

#include "Value.hpp"

class Environment
{
private:

    std::unordered_map<
        std::string,
        Value
    > variables;

    Environment* parent;

public:

    Environment(
        Environment* parent = nullptr
    );

    void define(
    const std::string& name,
    const Value& value
);

Value get(
    const std::string& name
);

void assign(
    const std::string& name,
    const Value& value
);

};