#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <functional>

class Environment;
class BlockStatement;
class ArrayValue;
class ObjectValue;
class FunctionValue; // Forward declare

enum class ValueType
{
    NUMBER,
    STRING,
    BOOLEAN,
    ARRAY,
    OBJECT,
    NULL_VALUE,
    UNDEFINED,
    FUNCTION
};

class Value
{
public:
    ValueType type;
    double numberValue;
    std::string stringValue;
    bool boolValue;
    std::shared_ptr<ArrayValue> arrayValue;
    std::shared_ptr<ObjectValue> objectValue;
    std::shared_ptr<FunctionValue> functionValue;

    Value()
        : type(ValueType::UNDEFINED),
          numberValue(0),
          boolValue(false)
    {
    }

    static Value Number(
        double value
    )
    {
        Value v;
        v.type = ValueType::NUMBER;
        v.numberValue = value;
        return v;
    }

    static Value String(
        const std::string& value
    )
    {
        Value v;
        v.type = ValueType::STRING;
        v.stringValue = value;
        return v;
    }

    static Value Boolean(
        bool value
    )
    {
        Value v;
        v.type = ValueType::BOOLEAN;
        v.boolValue = value;
        return v;
    }

    static Value Array(
        const std::shared_ptr<ArrayValue>& value
    )
    {
        Value v;
        v.type = ValueType::ARRAY;
        v.arrayValue = value;
        return v;
    }

    static Value Object(
        const std::shared_ptr<ObjectValue>& value
    )
    {
        Value v;
        v.type = ValueType::OBJECT;
        v.objectValue = value;
        return v;
    }

    static Value Null()
    {
        Value v;
        v.type = ValueType::NULL_VALUE;
        return v;
    }

    static Value Undefined()
    {
        return Value();
    }

    static Value Function(
        const std::shared_ptr<FunctionValue>& value
    );
};

class FunctionValue
{
public:
    std::string name;
    std::vector<std::string> parameters;
    BlockStatement* body;
    Environment* closure;
    std::function<Value(const std::vector<Value>&)> nativeHandler;

    FunctionValue(
        std::string name,
        std::vector<std::string> parameters,
        BlockStatement* body,
        Environment* closure,
        std::function<Value(const std::vector<Value>&)> nativeHandler = nullptr
    )
        : name(std::move(name)),
          parameters(std::move(parameters)),
          body(body),
          closure(closure),
          nativeHandler(std::move(nativeHandler))
    {
    }
};

// Define inline method that requires FunctionValue definition
inline Value Value::Function(const std::shared_ptr<FunctionValue>& value)
{
    Value v;
    v.type = ValueType::FUNCTION;
    v.functionValue = value;
    return v;
}

class ArrayValue
{
public:
    std::vector<Value> elements;
};

class ObjectValue
{
public:
    std::unordered_map<
        std::string,
        Value
    > properties;
};
