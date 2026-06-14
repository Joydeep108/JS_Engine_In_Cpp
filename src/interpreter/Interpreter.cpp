#include "Interpreter.hpp"

#include <iostream>
#include <stdexcept>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <limits>
#include <cctype>
#include <sstream>
#include <functional>
#include <cstdlib>
#include <ctime>

namespace
{
    struct ReturnSignal
    {
        Value value;
    };

    struct BreakSignal
    {
    };

    struct JavaScriptException
    {
        Value value;
    };

    std::string numberToString(double val)
    {
        if(std::isnan(val)) return "NaN";
        if(std::isinf(val)) return val > 0 ? "Infinity" : "-Infinity";
        if(val == static_cast<long long>(val))
        {
            return std::to_string(static_cast<long long>(val));
        }
        std::string s = std::to_string(val);
        s.erase(s.find_last_not_of('0') + 1, std::string::npos);
        if(s.back() == '.')
        {
            s.pop_back();
        }
        return s;
    }

    bool isTruthy(
        const Value& value
    )
    {
        if(value.type == ValueType::BOOLEAN)
        {
            return value.boolValue;
        }

        if(value.type == ValueType::NUMBER)
        {
            return value.numberValue != 0 && !std::isnan(value.numberValue);
        }

        if(value.type == ValueType::STRING)
        {
            return !value.stringValue.empty();
        }

        if(value.type == ValueType::NULL_VALUE)
        {
            return false;
        }

        if(value.type == ValueType::UNDEFINED)
        {
            return false;
        }

        return true;
    }

    std::string valueToString(const Value& value)
    {
        if(value.type == ValueType::NUMBER)
        {
            return numberToString(value.numberValue);
        }
        if(value.type == ValueType::STRING)
        {
            return value.stringValue;
        }
        if(value.type == ValueType::BOOLEAN)
        {
            return value.boolValue ? "true" : "false";
        }
        if(value.type == ValueType::ARRAY)
        {
            std::string res = "[";
            if(value.arrayValue)
            {
                for(size_t i = 0; i < value.arrayValue->elements.size(); ++i)
                {
                    if(i > 0) res += ", ";
                    res += valueToString(value.arrayValue->elements[i]);
                }
            }
            res += "]";
            return res;
        }
        if(value.type == ValueType::OBJECT)
        {
            std::string res = "{";
            if(value.objectValue)
            {
                bool first = true;
                for(const auto& entry : value.objectValue->properties)
                {
                    if(!first) res += ", ";
                    res += entry.first + ": " + valueToString(entry.second);
                    first = false;
                }
            }
            res += "}";
            return res;
        }
        if(value.type == ValueType::NULL_VALUE)
        {
            return "null";
        }
        if(value.type == ValueType::UNDEFINED)
        {
            return "undefined";
        }
        if(value.type == ValueType::FUNCTION)
        {
            if(value.functionValue && !value.functionValue->name.empty())
            {
                return "[Function: " + value.functionValue->name + "]";
            }
            return "[Function]";
        }
        return "";
    }

    bool valuesEqual(
        const Value& left,
        const Value& right
    )
    {
        if(left.type != right.type)
        {
            return false;
        }

        if(left.type == ValueType::NUMBER)
        {
            if(std::isnan(left.numberValue) && std::isnan(right.numberValue)) return false;
            return left.numberValue == right.numberValue;
        }

        if(left.type == ValueType::STRING)
        {
            return left.stringValue == right.stringValue;
        }

        if(left.type == ValueType::BOOLEAN)
        {
            return left.boolValue == right.boolValue;
        }

        if(left.type == ValueType::NULL_VALUE)
        {
            return true;
        }

        if(left.type == ValueType::UNDEFINED)
        {
            return true;
        }

        if(left.type == ValueType::ARRAY)
        {
            return left.arrayValue == right.arrayValue;
        }

        if(left.type == ValueType::OBJECT)
        {
            return left.objectValue == right.objectValue;
        }

        if(left.type == ValueType::FUNCTION)
        {
            return left.functionValue == right.functionValue;
        }

        return false;
    }

    bool looseEqual(const Value& left, const Value& right)
    {
        if(left.type == right.type)
        {
            return valuesEqual(left, right);
        }
        if(left.type == ValueType::NULL_VALUE && right.type == ValueType::UNDEFINED) return true;
        if(left.type == ValueType::UNDEFINED && right.type == ValueType::NULL_VALUE) return true;
        if(left.type == ValueType::NUMBER && right.type == ValueType::STRING)
        {
            try { return left.numberValue == std::stod(right.stringValue); } catch(...) { return false; }
        }
        if(left.type == ValueType::STRING && right.type == ValueType::NUMBER)
        {
            try { return std::stod(left.stringValue) == right.numberValue; } catch(...) { return false; }
        }
        if(left.type == ValueType::BOOLEAN)
        {
            return looseEqual(Value::Number(left.boolValue ? 1 : 0), right);
        }
        if(right.type == ValueType::BOOLEAN)
        {
            return looseEqual(left, Value::Number(right.boolValue ? 1 : 0));
        }
        return false;
    }

    void printValue(
        const Value& value
    )
    {
        std::cout << valueToString(value);
    }

    enum class LValueKind { IDENTIFIER, INDEX, MEMBER };

    struct LValue
    {
        LValueKind kind;
        std::string name;
        Value arrayVal;
        int index;
        Value objectVal;
        std::string prop;
    };

    LValue evaluateLValue(Expression* expr, Interpreter* self)
    {
        if(auto id = dynamic_cast<Identifier*>(expr))
        {
            return LValue{LValueKind::IDENTIFIER, id->name, {}, 0, {}, ""};
        }
        if(auto indexExpr = dynamic_cast<IndexExpression*>(expr))
        {
            Value obj = self->evaluate(indexExpr->object.get());
            Value idx = self->evaluate(indexExpr->index.get());
            if(obj.type != ValueType::ARRAY || !obj.arrayValue)
            {
                throw std::runtime_error("Index access is only supported on arrays");
            }
            if(idx.type != ValueType::NUMBER)
            {
                throw std::runtime_error("Array index must be a number");
            }
            int pos = static_cast<int>(idx.numberValue);
            return LValue{LValueKind::INDEX, "", obj, pos, {}, ""};
        }
        if(auto member = dynamic_cast<MemberExpression*>(expr))
        {
            Value obj = self->evaluate(member->object.get());
            if(obj.type != ValueType::OBJECT || !obj.objectValue)
            {
                throw std::runtime_error("Property access is only supported on objects");
            }
            return LValue{LValueKind::MEMBER, "", {}, 0, obj, member->property};
        }
        throw std::runtime_error("Invalid assignment/update target");
    }

    Value getLValue(const LValue& lval, Environment* env)
    {
        if(lval.kind == LValueKind::IDENTIFIER)
        {
            return env->get(lval.name);
        }
        if(lval.kind == LValueKind::INDEX)
        {
            if(lval.index < 0 || lval.index >= static_cast<int>(lval.arrayVal.arrayValue->elements.size()))
            {
                return Value::Undefined();
            }
            return lval.arrayVal.arrayValue->elements.at(lval.index);
        }
        if(lval.kind == LValueKind::MEMBER)
        {
            auto prop = lval.objectVal.objectValue->properties.find(lval.prop);
            if(prop == lval.objectVal.objectValue->properties.end())
            {
                return Value::Undefined();
            }
            return prop->second;
        }
        return Value::Undefined();
    }

    void setLValue(const LValue& lval, const Value& val, Environment* env)
    {
        if(lval.kind == LValueKind::IDENTIFIER)
        {
            env->assign(lval.name, val);
        }
        else if(lval.kind == LValueKind::INDEX)
        {
            if(lval.index < 0)
            {
                throw std::runtime_error("Array index out of range");
            }
            if(lval.index >= static_cast<int>(lval.arrayVal.arrayValue->elements.size()))
            {
                lval.arrayVal.arrayValue->elements.resize(lval.index + 1, Value::Undefined());
            }
            lval.arrayVal.arrayValue->elements.at(lval.index) = val;
        }
        else if(lval.kind == LValueKind::MEMBER)
        {
            lval.objectVal.objectValue->properties[lval.prop] = val;
        }
    }
}

Interpreter::Interpreter()
    : currentEnv(&global)
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    auto consoleObj = std::make_shared<ObjectValue>();
    consoleObj->properties["log"] = Value::Function(std::make_shared<FunctionValue>(
        "log", std::vector<std::string>{}, nullptr, nullptr,
        [](const std::vector<Value>& args) -> Value {
            for(size_t i = 0; i < args.size(); ++i) {
                if(i > 0) std::cout << " ";
                std::cout << valueToString(args[i]);
            }
            std::cout << "\n";
            return Value::Undefined();
        }
    ));
    global.define("console", Value::Object(consoleObj));

    global.define("Number", Value::Function(std::make_shared<FunctionValue>(
        "Number", std::vector<std::string>{}, nullptr, nullptr,
        [](const std::vector<Value>& args) -> Value {
            if(args.empty()) return Value::Number(0);
            if(args[0].type == ValueType::NUMBER) return args[0];
            if(args[0].type == ValueType::BOOLEAN) return Value::Number(args[0].boolValue ? 1 : 0);
            if(args[0].type == ValueType::NULL_VALUE) return Value::Number(0);
            if(args[0].type == ValueType::UNDEFINED) return Value::Number(std::numeric_limits<double>::quiet_NaN());
            if(args[0].type == ValueType::STRING) {
                try { return Value::Number(std::stod(args[0].stringValue)); }
                catch(...) { return Value::Number(std::numeric_limits<double>::quiet_NaN()); }
            }
            return Value::Number(0);
        }
    )));

    global.define("String", Value::Function(std::make_shared<FunctionValue>(
        "String", std::vector<std::string>{}, nullptr, nullptr,
        [](const std::vector<Value>& args) -> Value {
            if(args.empty()) return Value::String("");
            return Value::String(valueToString(args[0]));
        }
    )));

    global.define("Boolean", Value::Function(std::make_shared<FunctionValue>(
        "Boolean", std::vector<std::string>{}, nullptr, nullptr,
        [](const std::vector<Value>& args) -> Value {
            if(args.empty()) return Value::Boolean(false);
            return Value::Boolean(isTruthy(args[0]));
        }
    )));

    auto mathObj = std::make_shared<ObjectValue>();
    mathObj->properties["floor"] = Value::Function(std::make_shared<FunctionValue>(
        "floor", std::vector<std::string>{}, nullptr, nullptr,
        [](const std::vector<Value>& args) -> Value {
            double val = args.empty() ? 0 : args[0].numberValue;
            return Value::Number(std::floor(val));
        }
    ));
    mathObj->properties["ceil"] = Value::Function(std::make_shared<FunctionValue>(
        "ceil", std::vector<std::string>{}, nullptr, nullptr,
        [](const std::vector<Value>& args) -> Value {
            double val = args.empty() ? 0 : args[0].numberValue;
            return Value::Number(std::ceil(val));
        }
    ));
    mathObj->properties["round"] = Value::Function(std::make_shared<FunctionValue>(
        "round", std::vector<std::string>{}, nullptr, nullptr,
        [](const std::vector<Value>& args) -> Value {
            double val = args.empty() ? 0 : args[0].numberValue;
            return Value::Number(std::round(val));
        }
    ));
    mathObj->properties["max"] = Value::Function(std::make_shared<FunctionValue>(
        "max", std::vector<std::string>{}, nullptr, nullptr,
        [](const std::vector<Value>& args) -> Value {
            if(args.empty()) return Value::Number(-std::numeric_limits<double>::infinity());
            double m = args[0].numberValue;
            for(size_t i = 1; i < args.size(); ++i) {
                if(args[i].numberValue > m) m = args[i].numberValue;
            }
            return Value::Number(m);
        }
    ));
    mathObj->properties["min"] = Value::Function(std::make_shared<FunctionValue>(
        "min", std::vector<std::string>{}, nullptr, nullptr,
        [](const std::vector<Value>& args) -> Value {
            if(args.empty()) return Value::Number(std::numeric_limits<double>::infinity());
            double m = args[0].numberValue;
            for(size_t i = 1; i < args.size(); ++i) {
                if(args[i].numberValue < m) m = args[i].numberValue;
            }
            return Value::Number(m);
        }
    ));
    mathObj->properties["pow"] = Value::Function(std::make_shared<FunctionValue>(
        "pow", std::vector<std::string>{}, nullptr, nullptr,
        [](const std::vector<Value>& args) -> Value {
            double base = args.size() > 0 ? args[0].numberValue : 0;
            double exp = args.size() > 1 ? args[1].numberValue : 0;
            return Value::Number(std::pow(base, exp));
        }
    ));
    mathObj->properties["sqrt"] = Value::Function(std::make_shared<FunctionValue>(
        "sqrt", std::vector<std::string>{}, nullptr, nullptr,
        [](const std::vector<Value>& args) -> Value {
            double val = args.empty() ? 0 : args[0].numberValue;
            return Value::Number(std::sqrt(val));
        }
    ));
    mathObj->properties["abs"] = Value::Function(std::make_shared<FunctionValue>(
        "abs", std::vector<std::string>{}, nullptr, nullptr,
        [](const std::vector<Value>& args) -> Value {
            double val = args.empty() ? 0 : args[0].numberValue;
            return Value::Number(std::abs(val));
        }
    ));
    mathObj->properties["random"] = Value::Function(std::make_shared<FunctionValue>(
        "random", std::vector<std::string>{}, nullptr, nullptr,
        [](const std::vector<Value>& args) -> Value {
            double r = static_cast<double>(rand()) / RAND_MAX;
            return Value::Number(r);
        }
    ));
    global.define("Math", Value::Object(mathObj));

    auto dateFuncObj = std::make_shared<FunctionValue>(
        "Date", std::vector<std::string>{}, nullptr, nullptr,
        [](const std::vector<Value>& args) -> Value {
            return Value::String("Sun Jun 14 2026 21:23:38 GMT+0530");
        }
    );
    global.define("Date", Value::Function(dateFuncObj));
}

void Interpreter::execute(
    Program* program
)
{
    try
    {
        executeStatements(
            program->statements
        );
    }
    catch(const ReturnSignal&)
    {
        throw std::runtime_error(
            "Return statement outside function"
        );
    }
    catch(const BreakSignal&)
    {
        throw std::runtime_error(
            "Break statement outside loop or switch"
        );
    }
}

void Interpreter::executeStatements(
    const std::vector<
        std::unique_ptr<Statement>
    >& statements
)
{
    for(const auto& stmt : statements)
    {
        executeStatement(stmt.get());
    }
}

void Interpreter::executeStatement(
    Statement* stmt
)
{
    if(auto varDecl =
        dynamic_cast<
            VariableDeclaration*
        >(stmt))
    {
        Value value = varDecl->initializer
            ? evaluate(varDecl->initializer.get())
            : Value::Undefined();

        currentEnv->define(
            varDecl->name,
            value
        );

        return;
    }

    if(auto functionDecl =
        dynamic_cast<
            FunctionDeclaration*
        >(stmt))
    {
        auto function =
            std::make_shared<
                FunctionValue
            >(
                functionDecl->name,
                functionDecl->parameters,
                functionDecl->body.get(),
                currentEnv
            );

        currentEnv->define(
            functionDecl->name,
            Value::Function(function)
        );

        return;
    }

    if(auto returnStmt =
        dynamic_cast<
            ReturnStatement*
        >(stmt))
    {
        Value returnValue =
            returnStmt->value
            ? evaluate(returnStmt->value.get())
            : Value::Undefined();

        throw ReturnSignal{
            returnValue
        };
    }

    if(dynamic_cast<
        BreakStatement*
    >(stmt))
    {
        throw BreakSignal{};
    }

    if(auto exprStmt =
        dynamic_cast<
            ExpressionStatement*
        >(stmt))
    {
        evaluate(
            exprStmt->expression.get()
        );

        return;
    }

    if(auto ifStmt =
        dynamic_cast<
            IfStatement*
        >(stmt))
    {
        Value condition =
            evaluate(
                ifStmt->condition.get()
            );

        if(
            isTruthy(condition)
        )
        {
            executeBlock(
                ifStmt->thenBlock.get()
            );
        }
        else if(ifStmt->elseBlock)
        {
            executeBlock(
                ifStmt->elseBlock.get()
            );
        }

        return;
    }

    if(auto whileStmt =
        dynamic_cast<
            WhileStatement*
        >(stmt))
    {
        while(true)
        {
            Value condition =
                evaluate(
                    whileStmt->condition.get()
                );

            if(
                !isTruthy(condition)
            )
            {
                break;
            }

            try
            {
                executeBlock(
                    whileStmt->body.get()
                );
            }
            catch(const BreakSignal&)
            {
                break;
            }
        }

        return;
    }

    if(auto doWhileStmt =
        dynamic_cast<
            DoWhileStatement*
        >(stmt))
    {
        do
        {
            try
            {
                executeBlock(
                    doWhileStmt->body.get()
                );
            }
            catch(const BreakSignal&)
            {
                break;
            }
        }
        while(
            isTruthy(
                evaluate(
                    doWhileStmt->condition.get()
                )
            )
        );

        return;
    }

    if(auto forStmt =
        dynamic_cast<
            ForStatement*
        >(stmt))
    {
        Environment forEnv(currentEnv);
        Environment* previous = currentEnv;
        currentEnv = &forEnv;

        try
        {
            if(forStmt->initializer)
            {
                executeStatement(
                    forStmt->initializer.get()
                );
            }

            while(true)
            {
                if(forStmt->condition)
                {
                    Value condition =
                        evaluate(
                            forStmt
                                ->condition
                                .get()
                        );

                    if(
                        !isTruthy(condition)
                    )
                    {
                        break;
                    }
                }

                try
                {
                    executeBlock(
                        forStmt->body.get()
                    );
                }
                catch(const BreakSignal&)
                {
                    break;
                }

                if(forStmt->update)
                {
                    evaluate(
                        forStmt
                            ->update
                            .get()
                    );
                }
            }
        }
        catch(...)
        {
            currentEnv = previous;
            throw;
        }

        currentEnv = previous;
        return;
    }

    if(auto switchStmt =
        dynamic_cast<
            SwitchStatement*
        >(stmt))
    {
        Value discriminant =
            evaluate(
                switchStmt->discriminant.get()
            );

        int startIndex = -1;
        int defaultIndex = -1;

        for(size_t index = 0;
            index < switchStmt->cases.size();
            ++index)
        {
            auto& switchCase =
                switchStmt->cases[index];

            if(!switchCase->test)
            {
                if(defaultIndex == -1)
                {
                    defaultIndex =
                        static_cast<int>(index);
                }

                continue;
            }

            if(
                valuesEqual(
                    discriminant,
                    evaluate(
                        switchCase->test.get()
                    )
                )
            )
            {
                startIndex =
                    static_cast<int>(index);
                break;
            }
        }

        if(startIndex == -1)
        {
            startIndex = defaultIndex;
        }

        if(startIndex == -1)
        {
            return;
        }

        Environment switchEnv(
            currentEnv
        );

        Environment* previous =
            currentEnv;

        currentEnv =
            &switchEnv;

        try
        {
            for(size_t index = startIndex;
                index < switchStmt->cases.size();
                ++index)
            {
                for(const auto& caseStmt :
                    switchStmt->cases[index]
                        ->statements)
                {
                    executeStatement(
                        caseStmt.get()
                    );
                }
            }
        }
        catch(const BreakSignal&)
        {
            currentEnv = previous;
            return;
        }
        catch(...)
        {
            currentEnv = previous;
            throw;
        }

        currentEnv = previous;
        return;
    }

    if(auto throwStmt =
        dynamic_cast<
            ThrowStatement*
        >(stmt))
    {
        Value errorVal = evaluate(throwStmt->argument.get());
        throw JavaScriptException{errorVal};
    }

    if(auto tryCatch =
        dynamic_cast<
            TryCatchStatement*
        >(stmt))
    {
        bool hasError = false;
        Value caughtError;

        try
        {
            executeBlock(tryCatch->tryBlock.get());
        }
        catch(const JavaScriptException& ex)
        {
            hasError = true;
            caughtError = ex.value;
        }
        catch(...)
        {
            if(tryCatch->finallyBlock)
            {
                executeBlock(tryCatch->finallyBlock.get());
            }
            throw;
        }

        if(hasError && tryCatch->catchBlock)
        {
            Environment catchEnv(currentEnv);
            if(!tryCatch->catchParam.empty())
            {
                catchEnv.define(tryCatch->catchParam, caughtError);
            }
            Environment* previous = currentEnv;
            currentEnv = &catchEnv;
            try
            {
                executeBlock(tryCatch->catchBlock.get());
            }
            catch(...)
            {
                currentEnv = previous;
                if(tryCatch->finallyBlock)
                {
                    executeBlock(tryCatch->finallyBlock.get());
                }
                throw;
            }
            currentEnv = previous;
        }

        if(tryCatch->finallyBlock)
        {
            executeBlock(tryCatch->finallyBlock.get());
        }
        return;
    }
}

Value Interpreter::evaluate(
    Expression* expr
)
{
    if(!expr)
    {
        return Value::Undefined();
    }

    if(auto num =
        dynamic_cast<
            NumberLiteral*
        >(expr))
    {
        return Value::Number(
            num->value
        );
    }

    if(auto str =
        dynamic_cast<
            StringLiteral*
        >(expr))
    {
        return Value::String(
            str->value
        );
    }

    if(dynamic_cast<
        NullLiteral*
    >(expr))
    {
        return Value::Null();
    }

    if(dynamic_cast<
        UndefinedLiteral*
    >(expr))
    {
        return Value::Undefined();
    }

    if(auto id =
        dynamic_cast<
            Identifier*
        >(expr))
    {
        return currentEnv->get(
            id->name
        );
    }

    if(auto boolean =
        dynamic_cast<
            BooleanLiteral*
        >(expr))
    {
        return Value::Boolean(
            boolean->value
        );
    }

    if(auto unary =
        dynamic_cast<
            UnaryExpression*
        >(expr))
    {
        Value argument =
            evaluate(
                unary->argument.get()
            );

        if(unary->op == "!")
        {
            return Value::Boolean(
                !isTruthy(argument)
            );
        }

        if(unary->op == "-")
        {
            if(argument.type != ValueType::NUMBER)
            {
                throw std::runtime_error(
                    "Unary '-' expects a number"
                );
            }

            return Value::Number(
                -argument.numberValue
            );
        }

        throw std::runtime_error(
            "Unknown unary operator"
        );
    }

    if(auto update =
        dynamic_cast<
            UpdateExpression*
        >(expr))
    {
        LValue lval = evaluateLValue(update->argument.get(), this);
        Value currentVal = getLValue(lval, currentEnv);
        if(currentVal.type != ValueType::NUMBER)
        {
            throw std::runtime_error("Increment/decrement operator expects a number");
        }
        double oldNum = currentVal.numberValue;
        double newNum = (update->op == "++") ? oldNum + 1 : oldNum - 1;
        Value newVal = Value::Number(newNum);
        setLValue(lval, newVal, currentEnv);
        return update->prefix ? newVal : Value::Number(oldNum);
    }

    if(auto compAssign =
        dynamic_cast<
            CompoundAssignmentExpression*
        >(expr))
    {
        LValue lval = evaluateLValue(compAssign->left.get(), this);
        Value currentVal = getLValue(lval, currentEnv);
        Value rightVal = evaluate(compAssign->right.get());
        Value newVal;
        if(compAssign->op == "+=")
        {
            if(currentVal.type == ValueType::STRING || rightVal.type == ValueType::STRING)
            {
                newVal = Value::String(valueToString(currentVal) + valueToString(rightVal));
            }
            else
            {
                newVal = Value::Number(currentVal.numberValue + rightVal.numberValue);
            }
        }
        else if(compAssign->op == "-=")
        {
            newVal = Value::Number(currentVal.numberValue - rightVal.numberValue);
        }
        else
        {
            throw std::runtime_error("Unknown compound assignment operator");
        }
        setLValue(lval, newVal, currentEnv);
        return newVal;
    }

    if(auto array =
        dynamic_cast<
            ArrayLiteral*
        >(expr))
    {
        auto elements =
            std::make_shared<
                ArrayValue
            >();

        for(auto& element :
            array->elements)
        {
            if(auto spread = dynamic_cast<SpreadElement*>(element.get()))
            {
                Value spreadVal = evaluate(spread->argument.get());
                if(spreadVal.type != ValueType::ARRAY || !spreadVal.arrayValue)
                {
                    throw std::runtime_error("Spread operator expects an array");
                }
                for(const auto& el : spreadVal.arrayValue->elements)
                {
                    elements->elements.push_back(el);
                }
            }
            else
            {
                elements->elements.push_back(
                    evaluate(element.get())
                );
            }
        }

        return Value::Array(elements);
    }

    if(auto object =
        dynamic_cast<
            ObjectLiteral*
        >(expr))
    {
        auto properties =
            std::make_shared<
                ObjectValue
            >();

        for(auto& property :
            object->properties)
        {
            properties->properties[property.key] =
                evaluate(property.value.get());
        }

        return Value::Object(properties);
    }

    if(auto member =
        dynamic_cast<
            MemberExpression*
        >(expr))
    {
        Value object =
            evaluate(
                member->object.get()
            );

        if(object.type == ValueType::ARRAY && object.arrayValue)
        {
            if(member->property == "length")
            {
                return Value::Number(object.arrayValue->elements.size());
            }

            std::string prop = member->property;
            if(prop == "push")
            {
                return Value::Function(std::make_shared<FunctionValue>(
                    "push", std::vector<std::string>{}, nullptr, nullptr,
                    [arrayVal = object.arrayValue](const std::vector<Value>& args) -> Value {
                        for(const auto& arg : args) {
                            arrayVal->elements.push_back(arg);
                        }
                        return Value::Number(arrayVal->elements.size());
                    }
                ));
            }
            if(prop == "pop")
            {
                return Value::Function(std::make_shared<FunctionValue>(
                    "pop", std::vector<std::string>{}, nullptr, nullptr,
                    [arrayVal = object.arrayValue](const std::vector<Value>& args) -> Value {
                        if(arrayVal->elements.empty()) return Value::Undefined();
                        Value last = arrayVal->elements.back();
                        arrayVal->elements.pop_back();
                        return last;
                    }
                ));
            }
            if(prop == "shift")
            {
                return Value::Function(std::make_shared<FunctionValue>(
                    "shift", std::vector<std::string>{}, nullptr, nullptr,
                    [arrayVal = object.arrayValue](const std::vector<Value>& args) -> Value {
                        if(arrayVal->elements.empty()) return Value::Undefined();
                        Value first = arrayVal->elements.front();
                        arrayVal->elements.erase(arrayVal->elements.begin());
                        return first;
                    }
                ));
            }
            if(prop == "unshift")
            {
                return Value::Function(std::make_shared<FunctionValue>(
                    "unshift", std::vector<std::string>{}, nullptr, nullptr,
                    [arrayVal = object.arrayValue](const std::vector<Value>& args) -> Value {
                        arrayVal->elements.insert(arrayVal->elements.begin(), args.begin(), args.end());
                        return Value::Number(arrayVal->elements.size());
                    }
                ));
            }
            if(prop == "slice")
            {
                return Value::Function(std::make_shared<FunctionValue>(
                    "slice", std::vector<std::string>{}, nullptr, nullptr,
                    [arrayVal = object.arrayValue](const std::vector<Value>& args) -> Value {
                        int size = static_cast<int>(arrayVal->elements.size());
                        int start = 0;
                        if(!args.empty() && args[0].type == ValueType::NUMBER) {
                            start = static_cast<int>(args[0].numberValue);
                            if(start < 0) start = std::max(0, size + start);
                        }
                        int end = size;
                        if(args.size() > 1 && args[1].type == ValueType::NUMBER) {
                            end = static_cast<int>(args[1].numberValue);
                            if(end < 0) end = std::max(0, size + end);
                        }
                        start = std::min(size, std::max(0, start));
                        end = std::min(size, std::max(start, end));
                        auto res = std::make_shared<ArrayValue>();
                        for(int i = start; i < end; ++i) {
                            res->elements.push_back(arrayVal->elements[i]);
                        }
                        return Value::Array(res);
                    }
                ));
            }
            if(prop == "splice")
            {
                return Value::Function(std::make_shared<FunctionValue>(
                    "splice", std::vector<std::string>{}, nullptr, nullptr,
                    [arrayVal = object.arrayValue](const std::vector<Value>& args) -> Value {
                        int size = static_cast<int>(arrayVal->elements.size());
                        int start = 0;
                        if(!args.empty() && args[0].type == ValueType::NUMBER) {
                            start = static_cast<int>(args[0].numberValue);
                            if(start < 0) start = std::max(0, size + start);
                            start = std::min(size, start);
                        }
                        int deleteCount = size - start;
                        if(args.size() > 1 && args[1].type == ValueType::NUMBER) {
                            deleteCount = static_cast<int>(args[1].numberValue);
                            deleteCount = std::min(size - start, std::max(0, deleteCount));
                        }
                        auto deleted = std::make_shared<ArrayValue>();
                        for(int i = 0; i < deleteCount; ++i) {
                            deleted->elements.push_back(arrayVal->elements[start + i]);
                        }
                        arrayVal->elements.erase(arrayVal->elements.begin() + start, arrayVal->elements.begin() + start + deleteCount);
                        if(args.size() > 2) {
                            arrayVal->elements.insert(arrayVal->elements.begin() + start, args.begin() + 2, args.end());
                        }
                        return Value::Array(deleted);
                    }
                ));
            }
            if(prop == "concat")
            {
                return Value::Function(std::make_shared<FunctionValue>(
                    "concat", std::vector<std::string>{}, nullptr, nullptr,
                    [arrayVal = object.arrayValue](const std::vector<Value>& args) -> Value {
                        auto res = std::make_shared<ArrayValue>(*arrayVal);
                        for(const auto& arg : args) {
                            if(arg.type == ValueType::ARRAY && arg.arrayValue) {
                                res->elements.insert(res->elements.end(), arg.arrayValue->elements.begin(), arg.arrayValue->elements.end());
                            } else {
                                res->elements.push_back(arg);
                            }
                        }
                        return Value::Array(res);
                    }
                ));
            }
            if(prop == "includes")
            {
                return Value::Function(std::make_shared<FunctionValue>(
                    "includes", std::vector<std::string>{}, nullptr, nullptr,
                    [arrayVal = object.arrayValue](const std::vector<Value>& args) -> Value {
                        if(args.empty()) return Value::Boolean(false);
                        for(const auto& el : arrayVal->elements) {
                            if(valuesEqual(el, args[0])) return Value::Boolean(true);
                        }
                        return Value::Boolean(false);
                    }
                ));
            }
            if(prop == "indexOf")
            {
                return Value::Function(std::make_shared<FunctionValue>(
                    "indexOf", std::vector<std::string>{}, nullptr, nullptr,
                    [arrayVal = object.arrayValue](const std::vector<Value>& args) -> Value {
                        if(args.empty()) return Value::Number(-1);
                        for(size_t i = 0; i < arrayVal->elements.size(); ++i) {
                            if(valuesEqual(arrayVal->elements[i], args[0])) return Value::Number(static_cast<double>(i));
                        }
                        return Value::Number(-1);
                    }
                ));
            }
            if(prop == "sort")
            {
                return Value::Function(std::make_shared<FunctionValue>(
                    "sort", std::vector<std::string>{}, nullptr, nullptr,
                    [this, arrayVal = object.arrayValue](const std::vector<Value>& args) -> Value {
                        if(!args.empty() && args[0].type == ValueType::FUNCTION && args[0].functionValue) {
                            auto comp = args[0].functionValue;
                            std::stable_sort(arrayVal->elements.begin(), arrayVal->elements.end(), [this, comp](const Value& a, const Value& b) {
                                Value res = this->callFunction(comp, {a, b});
                                return res.numberValue < 0;
                            });
                        } else {
                            std::stable_sort(arrayVal->elements.begin(), arrayVal->elements.end(), [](const Value& a, const Value& b) {
                                return valueToString(a) < valueToString(b);
                            });
                        }
                        return Value::Array(arrayVal);
                    }
                ));
            }
            if(prop == "reverse")
            {
                return Value::Function(std::make_shared<FunctionValue>(
                    "reverse", std::vector<std::string>{}, nullptr, nullptr,
                    [arrayVal = object.arrayValue](const std::vector<Value>& args) -> Value {
                        std::reverse(arrayVal->elements.begin(), arrayVal->elements.end());
                        return Value::Array(arrayVal);
                    }
                ));
            }
            if(prop == "join")
            {
                return Value::Function(std::make_shared<FunctionValue>(
                    "join", std::vector<std::string>{}, nullptr, nullptr,
                    [arrayVal = object.arrayValue](const std::vector<Value>& args) -> Value {
                        std::string sep = ",";
                        if(!args.empty() && args[0].type == ValueType::STRING) {
                            sep = args[0].stringValue;
                        }
                        std::string res = "";
                        for(size_t i = 0; i < arrayVal->elements.size(); ++i) {
                            if(i > 0) res += sep;
                            res += valueToString(arrayVal->elements[i]);
                        }
                        return Value::String(res);
                    }
                ));
            }
            if(prop == "map")
            {
                return Value::Function(std::make_shared<FunctionValue>(
                    "map", std::vector<std::string>{}, nullptr, nullptr,
                    [this, arrayVal = object.arrayValue](const std::vector<Value>& args) -> Value {
                        if(args.empty() || args[0].type != ValueType::FUNCTION || !args[0].functionValue) {
                            throw std::runtime_error("map expects a callback function");
                        }
                        auto cb = args[0].functionValue;
                        auto res = std::make_shared<ArrayValue>();
                        for(size_t i = 0; i < arrayVal->elements.size(); ++i) {
                            Value mapped = this->callFunction(cb, {arrayVal->elements[i], Value::Number(static_cast<double>(i)), Value::Array(arrayVal)});
                            res->elements.push_back(mapped);
                        }
                        return Value::Array(res);
                    }
                ));
            }
            if(prop == "filter")
            {
                return Value::Function(std::make_shared<FunctionValue>(
                    "filter", std::vector<std::string>{}, nullptr, nullptr,
                    [this, arrayVal = object.arrayValue](const std::vector<Value>& args) -> Value {
                        if(args.empty() || args[0].type != ValueType::FUNCTION || !args[0].functionValue) {
                            throw std::runtime_error("filter expects a callback function");
                        }
                        auto cb = args[0].functionValue;
                        auto res = std::make_shared<ArrayValue>();
                        for(size_t i = 0; i < arrayVal->elements.size(); ++i) {
                            Value keep = this->callFunction(cb, {arrayVal->elements[i], Value::Number(static_cast<double>(i)), Value::Array(arrayVal)});
                            if(isTruthy(keep)) {
                                res->elements.push_back(arrayVal->elements[i]);
                            }
                        }
                        return Value::Array(res);
                    }
                ));
            }
            if(prop == "reduce")
            {
                return Value::Function(std::make_shared<FunctionValue>(
                    "reduce", std::vector<std::string>{}, nullptr, nullptr,
                    [this, arrayVal = object.arrayValue](const std::vector<Value>& args) -> Value {
                        if(args.empty() || args[0].type != ValueType::FUNCTION || !args[0].functionValue) {
                            throw std::runtime_error("reduce expects a callback function");
                        }
                        auto cb = args[0].functionValue;
                        if(arrayVal->elements.empty() && args.size() < 2) {
                            throw std::runtime_error("reduce of empty array with no initial value");
                        }
                        Value accum;
                        size_t start = 0;
                        if(args.size() >= 2) {
                            accum = args[1];
                        } else {
                            accum = arrayVal->elements[0];
                            start = 1;
                        }
                        for(size_t i = start; i < arrayVal->elements.size(); ++i) {
                            accum = this->callFunction(cb, {accum, arrayVal->elements[i], Value::Number(static_cast<double>(i)), Value::Array(arrayVal)});
                        }
                        return accum;
                    }
                ));
            }
            if(prop == "find")
            {
                return Value::Function(std::make_shared<FunctionValue>(
                    "find", std::vector<std::string>{}, nullptr, nullptr,
                    [this, arrayVal = object.arrayValue](const std::vector<Value>& args) -> Value {
                        if(args.empty() || args[0].type != ValueType::FUNCTION || !args[0].functionValue) {
                            throw std::runtime_error("find expects a callback function");
                        }
                        auto cb = args[0].functionValue;
                        for(size_t i = 0; i < arrayVal->elements.size(); ++i) {
                            Value match = this->callFunction(cb, {arrayVal->elements[i], Value::Number(static_cast<double>(i)), Value::Array(arrayVal)});
                            if(isTruthy(match)) {
                                return arrayVal->elements[i];
                            }
                        }
                        return Value::Undefined();
                    }
                ));
            }
            if(prop == "some")
            {
                return Value::Function(std::make_shared<FunctionValue>(
                    "some", std::vector<std::string>{}, nullptr, nullptr,
                    [this, arrayVal = object.arrayValue](const std::vector<Value>& args) -> Value {
                        if(args.empty() || args[0].type != ValueType::FUNCTION || !args[0].functionValue) {
                            throw std::runtime_error("some expects a callback function");
                        }
                        auto cb = args[0].functionValue;
                        for(size_t i = 0; i < arrayVal->elements.size(); ++i) {
                            Value match = this->callFunction(cb, {arrayVal->elements[i], Value::Number(static_cast<double>(i)), Value::Array(arrayVal)});
                            if(isTruthy(match)) {
                                return Value::Boolean(true);
                            }
                        }
                        return Value::Boolean(false);
                    }
                ));
            }
            if(prop == "every")
            {
                return Value::Function(std::make_shared<FunctionValue>(
                    "every", std::vector<std::string>{}, nullptr, nullptr,
                    [this, arrayVal = object.arrayValue](const std::vector<Value>& args) -> Value {
                        if(args.empty() || args[0].type != ValueType::FUNCTION || !args[0].functionValue) {
                            throw std::runtime_error("every expects a callback function");
                        }
                        auto cb = args[0].functionValue;
                        for(size_t i = 0; i < arrayVal->elements.size(); ++i) {
                            Value match = this->callFunction(cb, {arrayVal->elements[i], Value::Number(static_cast<double>(i)), Value::Array(arrayVal)});
                            if(!isTruthy(match)) {
                                return Value::Boolean(false);
                            }
                        }
                        return Value::Boolean(true);
                    }
                ));
            }
            if(prop == "forEach")
            {
                return Value::Function(std::make_shared<FunctionValue>(
                    "forEach", std::vector<std::string>{}, nullptr, nullptr,
                    [this, arrayVal = object.arrayValue](const std::vector<Value>& args) -> Value {
                        if(args.empty() || args[0].type != ValueType::FUNCTION || !args[0].functionValue) {
                            throw std::runtime_error("forEach expects a callback function");
                        }
                        auto cb = args[0].functionValue;
                        for(size_t i = 0; i < arrayVal->elements.size(); ++i) {
                            this->callFunction(cb, {arrayVal->elements[i], Value::Number(static_cast<double>(i)), Value::Array(arrayVal)});
                        }
                        return Value::Undefined();
                    }
                ));
            }
        }

        if(object.type == ValueType::STRING)
        {
            if(member->property == "length")
            {
                return Value::Number(object.stringValue.length());
            }

            std::string prop = member->property;
            if(prop == "split")
            {
                return Value::Function(std::make_shared<FunctionValue>(
                    "split", std::vector<std::string>{}, nullptr, nullptr,
                    [str = object.stringValue](const std::vector<Value>& args) -> Value {
                        std::string sep = "";
                        if(!args.empty() && args[0].type == ValueType::STRING) {
                            sep = args[0].stringValue;
                        }
                        auto res = std::make_shared<ArrayValue>();
                        if(sep.empty()) {
                            for(char c : str) {
                                res->elements.push_back(Value::String(std::string(1, c)));
                            }
                        } else {
                            size_t start = 0;
                            size_t end = str.find(sep);
                            while(end != std::string::npos) {
                                res->elements.push_back(Value::String(str.substr(start, end - start)));
                                start = end + sep.length();
                                end = str.find(sep, start);
                            }
                            res->elements.push_back(Value::String(str.substr(start)));
                        }
                        return Value::Array(res);
                    }
                ));
            }
            if(prop == "replace")
            {
                return Value::Function(std::make_shared<FunctionValue>(
                    "replace", std::vector<std::string>{}, nullptr, nullptr,
                    [str = object.stringValue](const std::vector<Value>& args) -> Value {
                        if(args.size() < 2 || args[0].type != ValueType::STRING || args[1].type != ValueType::STRING) {
                            return Value::String(str);
                        }
                        std::string pat = args[0].stringValue;
                        std::string rep = args[1].stringValue;
                        size_t pos = str.find(pat);
                        if(pos == std::string::npos) return Value::String(str);
                        std::string res = str;
                        res.replace(pos, pat.length(), rep);
                        return Value::String(res);
                    }
                ));
            }
            if(prop == "replaceAll")
            {
                return Value::Function(std::make_shared<FunctionValue>(
                    "replaceAll", std::vector<std::string>{}, nullptr, nullptr,
                    [str = object.stringValue](const std::vector<Value>& args) -> Value {
                        if(args.size() < 2 || args[0].type != ValueType::STRING || args[1].type != ValueType::STRING) {
                            return Value::String(str);
                        }
                        std::string pat = args[0].stringValue;
                        std::string rep = args[1].stringValue;
                        if(pat.empty()) return Value::String(str);
                        std::string res = "";
                        size_t start = 0;
                        size_t end = str.find(pat);
                        while(end != std::string::npos) {
                            res += str.substr(start, end - start) + rep;
                            start = end + pat.length();
                            end = str.find(pat, start);
                        }
                        res += str.substr(start);
                        return Value::String(res);
                    }
                ));
            }
            if(prop == "substring")
            {
                return Value::Function(std::make_shared<FunctionValue>(
                    "substring", std::vector<std::string>{}, nullptr, nullptr,
                    [str = object.stringValue](const std::vector<Value>& args) -> Value {
                        int len = static_cast<int>(str.length());
                        int start = 0;
                        if(!args.empty() && args[0].type == ValueType::NUMBER) {
                            start = std::min(len, std::max(0, static_cast<int>(args[0].numberValue)));
                        }
                        int end = len;
                        if(args.size() > 1 && args[1].type == ValueType::NUMBER) {
                            end = std::min(len, std::max(0, static_cast<int>(args[1].numberValue)));
                        }
                        if(start > end) std::swap(start, end);
                        return Value::String(str.substr(start, end - start));
                    }
                ));
            }
            if(prop == "slice")
            {
                return Value::Function(std::make_shared<FunctionValue>(
                    "slice", std::vector<std::string>{}, nullptr, nullptr,
                    [str = object.stringValue](const std::vector<Value>& args) -> Value {
                        int len = static_cast<int>(str.length());
                        int start = 0;
                        if(!args.empty() && args[0].type == ValueType::NUMBER) {
                            start = static_cast<int>(args[0].numberValue);
                            if(start < 0) start = std::max(0, len + start);
                        }
                        int end = len;
                        if(args.size() > 1 && args[1].type == ValueType::NUMBER) {
                            end = static_cast<int>(args[1].numberValue);
                            if(end < 0) end = std::max(0, len + end);
                        }
                        start = std::min(len, std::max(0, start));
                        end = std::min(len, std::max(start, end));
                        return Value::String(str.substr(start, end - start));
                    }
                ));
            }
            if(prop == "trim")
            {
                return Value::Function(std::make_shared<FunctionValue>(
                    "trim", std::vector<std::string>{}, nullptr, nullptr,
                    [str = object.stringValue](const std::vector<Value>& args) -> Value {
                        std::string s = str;
                        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
                            return !std::isspace(ch);
                        }));
                        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
                            return !std::isspace(ch);
                        }).base(), s.end());
                        return Value::String(s);
                    }
                ));
            }
            if(prop == "toUpperCase")
            {
                return Value::Function(std::make_shared<FunctionValue>(
                    "toUpperCase", std::vector<std::string>{}, nullptr, nullptr,
                    [str = object.stringValue](const std::vector<Value>& args) -> Value {
                        std::string s = str;
                        for(char& c : s) c = std::toupper(static_cast<unsigned char>(c));
                        return Value::String(s);
                    }
                ));
            }
            if(prop == "toLowerCase")
            {
                return Value::Function(std::make_shared<FunctionValue>(
                    "toLowerCase", std::vector<std::string>{}, nullptr, nullptr,
                    [str = object.stringValue](const std::vector<Value>& args) -> Value {
                        std::string s = str;
                        for(char& c : s) c = std::tolower(static_cast<unsigned char>(c));
                        return Value::String(s);
                    }
                ));
            }
            if(prop == "includes")
            {
                return Value::Function(std::make_shared<FunctionValue>(
                    "includes", std::vector<std::string>{}, nullptr, nullptr,
                    [str = object.stringValue](const std::vector<Value>& args) -> Value {
                        if(args.empty() || args[0].type != ValueType::STRING) return Value::Boolean(false);
                        return Value::Boolean(str.find(args[0].stringValue) != std::string::npos);
                    }
                ));
            }
            if(prop == "startsWith")
            {
                return Value::Function(std::make_shared<FunctionValue>(
                    "startsWith", std::vector<std::string>{}, nullptr, nullptr,
                    [str = object.stringValue](const std::vector<Value>& args) -> Value {
                        if(args.empty() || args[0].type != ValueType::STRING) return Value::Boolean(false);
                        return Value::Boolean(str.rfind(args[0].stringValue, 0) == 0);
                    }
                ));
            }
            if(prop == "endsWith")
            {
                return Value::Function(std::make_shared<FunctionValue>(
                    "endsWith", std::vector<std::string>{}, nullptr, nullptr,
                    [str = object.stringValue](const std::vector<Value>& args) -> Value {
                        if(args.empty() || args[0].type != ValueType::STRING) return Value::Boolean(false);
                        std::string pat = args[0].stringValue;
                        if(pat.length() > str.length()) return Value::Boolean(false);
                        return Value::Boolean(str.compare(str.length() - pat.length(), pat.length(), pat) == 0);
                    }
                ));
            }
            if(prop == "indexOf")
            {
                return Value::Function(std::make_shared<FunctionValue>(
                    "indexOf", std::vector<std::string>{}, nullptr, nullptr,
                    [str = object.stringValue](const std::vector<Value>& args) -> Value {
                        if(args.empty() || args[0].type != ValueType::STRING) return Value::Number(-1);
                        size_t pos = str.find(args[0].stringValue);
                        return Value::Number(pos == std::string::npos ? -1 : static_cast<double>(pos));
                    }
                ));
            }
            if(prop == "charAt")
            {
                return Value::Function(std::make_shared<FunctionValue>(
                    "charAt", std::vector<std::string>{}, nullptr, nullptr,
                    [str = object.stringValue](const std::vector<Value>& args) -> Value {
                        int pos = 0;
                        if(!args.empty() && args[0].type == ValueType::NUMBER) pos = static_cast<int>(args[0].numberValue);
                        if(pos < 0 || pos >= static_cast<int>(str.length())) return Value::String("");
                        return Value::String(std::string(1, str[pos]));
                    }
                ));
            }
        }

        if(object.type == ValueType::FUNCTION && object.functionValue)
        {
            if(object.functionValue->name == "Date" && member->property == "now")
            {
                return Value::Function(std::make_shared<FunctionValue>(
                    "now", std::vector<std::string>{}, nullptr, nullptr,
                    [](const std::vector<Value>& args) -> Value {
                        auto now = std::chrono::system_clock::now();
                        auto duration = now.time_since_epoch();
                        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
                        return Value::Number(static_cast<double>(millis));
                    }
                ));
            }
        }

        if(
            object.type != ValueType::OBJECT ||
            !object.objectValue
        )
        {
            throw std::runtime_error(
                "Property access is only supported on objects"
            );
        }

        auto property =
            object.objectValue->properties.find(
                member->property
            );

        if(
            property ==
            object.objectValue->properties.end()
        )
        {
            return Value::Undefined();
        }

        return property->second;
    }

    if(auto index =
        dynamic_cast<
            IndexExpression*
        >(expr))
    {
        Value object =
            evaluate(
                index->object.get()
            );

        Value indexValue =
            evaluate(
                index->index.get()
            );

        if(object.type == ValueType::STRING)
        {
            if(indexValue.type != ValueType::NUMBER)
            {
                throw std::runtime_error("String index must be a number");
            }
            int position = static_cast<int>(indexValue.numberValue);
            if(position < 0 || position >= static_cast<int>(object.stringValue.length()))
            {
                return Value::Undefined();
            }
            return Value::String(std::string(1, object.stringValue[position]));
        }

        if(
            object.type != ValueType::ARRAY ||
            !object.arrayValue
        )
        {
            throw std::runtime_error(
                "Index access is only supported on arrays and strings"
            );
        }

        if(indexValue.type != ValueType::NUMBER)
        {
            throw std::runtime_error(
                "Array index must be a number"
            );
        }

        int position =
            static_cast<int>(
                indexValue.numberValue
            );

        if(
            position < 0 ||
            position >= static_cast<int>(
                object.arrayValue->elements.size()
            )
        )
        {
            return Value::Undefined();
        }

        return object.arrayValue->elements.at(position);
    }

    if(auto assign =
        dynamic_cast<
            AssignmentExpression*
        >(expr))
    {
        LValue lval = evaluateLValue(assign->left.get(), this);
        Value value = evaluate(assign->value.get());
        setLValue(lval, value, currentEnv);
        return value;
    }

    if(auto call =
        dynamic_cast<
            CallExpression*
        >(expr))
    {
        auto member =
            dynamic_cast<
                MemberExpression*
            >(
                call->callee.get()
            );

        if(member)
        {
            auto object =
                dynamic_cast<
                    Identifier*
                >(
                    member->object.get()
                );

            if(
                object &&
                object->name == "console" &&
                member->property == "log"
            )
            {
                for(size_t i = 0; i < call->arguments.size(); ++i)
                {
                    if(i > 0) std::cout << " ";
                    printValue(
                        evaluate(call->arguments[i].get())
                    );
                }

                std::cout << '\n';

                return Value::Undefined();
            }
        }

        Value callee =
            evaluate(
                call->callee.get()
            );

        if(
            callee.type !=
                ValueType::FUNCTION
            ||
            !callee.functionValue
        )
        {
            throw std::runtime_error(
                "Can only call functions"
            );
        }

        std::vector<Value> arguments;

        for(auto& arg : call->arguments)
        {
            if(auto spread = dynamic_cast<SpreadElement*>(arg.get()))
            {
                Value spreadVal = evaluate(spread->argument.get());
                if(spreadVal.type != ValueType::ARRAY || !spreadVal.arrayValue)
                {
                    throw std::runtime_error("Spread operator expects an array");
                }
                for(const auto& val : spreadVal.arrayValue->elements)
                {
                    arguments.push_back(val);
                }
            }
            else
            {
                arguments.push_back(
                    evaluate(arg.get())
                );
            }
        }

        return callFunction(
            callee.functionValue,
            arguments
        );
    }

    if(auto fnExpr =
        dynamic_cast<
            FunctionExpression*
        >(expr))
    {
        auto function =
            std::make_shared<
                FunctionValue
            >(
                fnExpr->name,
                fnExpr->parameters,
                fnExpr->body.get(),
                currentEnv
            );
        return Value::Function(function);
    }

    if(auto arrowExpr =
        dynamic_cast<
            ArrowFunctionExpression*
        >(expr))
    {
        if(arrowExpr->expression)
        {
            std::vector<std::string> params = arrowExpr->parameters;
            Expression* arrowBodyExpr = arrowExpr->expression.get();
            Environment* closureEnv = currentEnv;

            auto native = [this, params, arrowBodyExpr, closureEnv](const std::vector<Value>& args) -> Value {
                Environment env(closureEnv);
                for(size_t i = 0; i < params.size(); ++i)
                {
                    Value arg = i < args.size() ? args[i] : Value::Undefined();
                    env.define(params[i], arg);
                }
                Environment* prevEnv = this->currentEnv;
                this->currentEnv = &env;
                try {
                    Value result = this->evaluate(arrowBodyExpr);
                    this->currentEnv = prevEnv;
                    return result;
                } catch(...) {
                    this->currentEnv = prevEnv;
                    throw;
                }
            };

            auto function = std::make_shared<FunctionValue>(
                "",
                params,
                nullptr,
                currentEnv,
                native
            );
            return Value::Function(function);
        }
        else
        {
            auto function =
                std::make_shared<
                    FunctionValue
                >(
                    "",
                    arrowExpr->parameters,
                    arrowExpr->body.get(),
                    currentEnv
                );
            return Value::Function(function);
        }
    }

    if(auto binary =
        dynamic_cast<
            BinaryExpression*
        >(expr))
    {
        Value left =
            evaluate(
                binary->left.get()
            );

        if(binary->op == "&&")
        {
            if(!isTruthy(left))
            {
                return left;
            }

            return evaluate(
                binary->right.get()
            );
        }

        if(binary->op == "||")
        {
            if(isTruthy(left))
            {
                return left;
            }

            return evaluate(
                binary->right.get()
            );
        }

        Value right =
            evaluate(
                binary->right.get()
            );

        if(binary->op == "+")
        {
            if(left.type == ValueType::STRING || right.type == ValueType::STRING)
            {
                return Value::String(valueToString(left) + valueToString(right));
            }
            return Value::Number(
                left.numberValue +
                right.numberValue
            );
        }

        if(binary->op == "-")
        {
            return Value::Number(
                left.numberValue -
                right.numberValue
            );
        }

        if(binary->op == "*")
        {
            return Value::Number(
                left.numberValue *
                right.numberValue
            );
        }

        if(binary->op == "/")
        {
            return Value::Number(
                left.numberValue /
                right.numberValue
            );
        }

        if(binary->op == "**")
        {
            return Value::Number(
                std::pow(left.numberValue, right.numberValue)
            );
        }

        if(binary->op == ">")
        {
            if(left.type == ValueType::STRING && right.type == ValueType::STRING)
            {
                return Value::Boolean(left.stringValue > right.stringValue);
            }
            return Value::Boolean(
                left.numberValue >
                right.numberValue
            );
        }

        if(binary->op == "<")
        {
            if(left.type == ValueType::STRING && right.type == ValueType::STRING)
            {
                return Value::Boolean(left.stringValue < right.stringValue);
            }
            return Value::Boolean(
                left.numberValue <
                right.numberValue
            );
        }

        if(binary->op == ">=")
        {
            if(left.type == ValueType::STRING && right.type == ValueType::STRING)
            {
                return Value::Boolean(left.stringValue >= right.stringValue);
            }
            return Value::Boolean(
                left.numberValue >=
                right.numberValue
            );
        }

        if(binary->op == "<=")
        {
            if(left.type == ValueType::STRING && right.type == ValueType::STRING)
            {
                return Value::Boolean(left.stringValue <= right.stringValue);
            }
            return Value::Boolean(
                left.numberValue <=
                right.numberValue
            );
        }

        if(binary->op == "==")
        {
            return Value::Boolean(
                looseEqual(left, right)
            );
        }

        if(binary->op == "===")
        {
            return Value::Boolean(
                valuesEqual(left, right)
            );
        }

        if(binary->op == "!=")
        {
            return Value::Boolean(
                !looseEqual(left, right)
            );
        }

        if(binary->op == "!==")
        {
            return Value::Boolean(
                !valuesEqual(left, right)
            );
        }

        if(binary->op == "%")
        {
            return Value::Number(
                static_cast<int>(left.numberValue) %
                static_cast<int>(right.numberValue)
            );
        }

        throw std::runtime_error(
            "Unknown expression"
        );
    }

    throw std::runtime_error(
        "Unsupported expression type"
    );
}

void Interpreter::executeBlock(
    BlockStatement* block
)
{
    if(!block)
    {
        return;
    }

    Environment* blockEnv = new Environment(currentEnv);

    Environment* previous =
        currentEnv;

    currentEnv =
        blockEnv;

    try
    {
        executeStatements(
            block->statements
        );
    }
    catch(...)
    {
        currentEnv = previous;
        throw;
    }

    currentEnv = previous;
}

Value Interpreter::callFunction(
    const std::shared_ptr<FunctionValue>& function,
    const std::vector<Value>& arguments
)
{
    if(function->nativeHandler)
    {
        return function->nativeHandler(arguments);
    }

    Environment* functionEnv = new Environment(function->closure);

    for(size_t index = 0;
        index < function->parameters.size();
        ++index)
    {
        Value argument =
            index < arguments.size()
            ? arguments[index]
            : Value::Undefined();

        functionEnv->define(
            function->parameters[index],
            argument
        );
    }

    Environment* previous =
        currentEnv;

    currentEnv =
        functionEnv;

    try
    {
        if(function->body)
        {
            executeStatements(
                function->body->statements
            );
        }
    }
    catch(const ReturnSignal& signal)
    {
        currentEnv = previous;
        return signal.value;
    }
    catch(const BreakSignal&)
    {
        currentEnv = previous;
        throw std::runtime_error(
            "Break statement outside loop or switch"
        );
    }
    catch(...)
    {
        currentEnv = previous;
        throw;
    }

    currentEnv = previous;
    return Value::Undefined();
}
