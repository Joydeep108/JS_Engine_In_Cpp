#include "Environment.hpp"
#include <stdexcept>

Environment::Environment(
    Environment* parent
)
    : parent(parent)
{
}

Value Environment::get(
    const std::string& name
)
{
    if(
        variables.find(name)
        != variables.end()
    )
    {
        return variables[name];
    }

    if(parent)
    {
        return parent->get(name);
    }

    throw std::runtime_error(
        "Undefined variable: " +
        name
    );
}

void Environment::assign(
    const std::string& name,
    const Value& value
)
{
    if(
        variables.find(name)
        != variables.end()
    )
    {
        variables[name] = value;
        return;
    }

    if(parent)
    {
        parent->assign(
            name,
            value
        );

        return;
    }

    throw std::runtime_error(
        "Undefined variable: " +
        name
    );
}

void Environment::define(
    const std::string& name,
    const Value& value
)
{
    variables[name] = value;
}
