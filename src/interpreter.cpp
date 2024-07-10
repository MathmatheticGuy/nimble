#include "interpreter.hpp"

Interpreter::Interpreter()
{
    globals->define("clock", std::shared_ptr<NativeClock>{});

    environment = globals;
}

void Interpreter::interpret(const std::vector<std::shared_ptr<Stmt>>& statements)
{
    try
    {
        for (const std::shared_ptr<Stmt>& statement : statements)
            execute(statement);
    }
    catch (RuntimeError error)
    {
        Error::runtime_error(error);
    }
}

std::string Interpreter::interpret(const std::shared_ptr<Expr>& expr)
{
    try
    {
        std::any value = evaluate(expr);
        return stringify(value);
    }
    catch(RuntimeError error)
    {
        Error::runtime_error(error);
        return "";
    }
    
}

std::any Interpreter::visitBlockStmt(std::shared_ptr<BlockStmt> stmt)
{
    execute_block(stmt->statements, std::make_shared<Environment>(environment));
    return {};
}

std::any Interpreter::visitExpressionStmt(std::shared_ptr<ExpressionStmt> stmt)
{
    evaluate(stmt->expression);
    return {};
}

std::any Interpreter::visitPrintStmt(std::shared_ptr<PrintStmt> stmt)
{
    std::any value = evaluate(stmt->expression);
    std::cout << stringify(value) + "\n";
    return {};
}

std::any Interpreter::visitVarStmt(std::shared_ptr<VarStmt> stmt)
{
    std::any value = nullptr;

    if (stmt->initializer != nullptr)
        value = evaluate(stmt->initializer);

    environment->define(stmt->name.lexeme, std::move(value));
    
    return {};
}

std::any Interpreter::visitIfStmt(std::shared_ptr<IfStmt> stmt)
{
    if (is_truthy(evaluate(stmt->condition)))
        execute(stmt->then_branch);
    else if (stmt->else_branch != nullptr)
        execute(stmt->else_branch);
    
    return {};
}

std::any Interpreter::visitWhileStmt(std::shared_ptr<WhileStmt> stmt)
{
    while (is_truthy(evaluate(stmt->condition)))
        execute(stmt->body);

    return {};
}


// std::any Interpreter::visitFunctionStmt(std::shared_ptr<FunctionStmt> stmt)
// {
//     auto function = std::make_shared<NblFunction>(stmt, environment);
//     environment->define(stmt->name.lexeme, function);
//     return {};
// }

// std::any Interpreter::visitReturnStmt(std::shared_ptr<ReturnStmt> stmt)
// {
//     std::any value = nullptr;

//     if (stmt->value != nullptr)
//         value = evaluate(stmt->value);

//     throw NblReturn{value};
// }

std::any Interpreter::visitAssignExpr(std::shared_ptr<AssignExpr> expr)
{
    std::any value = evaluate(expr->value);
    environment->assign(expr->name, value);
    return value;
}

std::any Interpreter::visitBinaryExpr(std::shared_ptr<BinaryExpr> expr)
{
    // evaluate binary operators
    std::any left = evaluate(expr->left);
    std::any right = evaluate(expr->right);

    switch (expr->op.type)
    {
        // comparisors
        case BANG_EQUAL: return !is_equal(left, right);
        case EQUAL_EQUAL: return is_equal(left, right);
        case GREATER:
            check_num_operands(expr->op, left, right);
            return std::any_cast<double>(left) > std::any_cast<double>(right);
        case GREATER_EQUAL:
            check_num_operands(expr->op, left, right);
            return std::any_cast<double>(left) >= std::any_cast<double>(right);
        case LESS:
            check_num_operands(expr->op, left, right);
            return std::any_cast<double>(left) < std::any_cast<double>(right);
        case LESS_EQUAL:
            check_num_operands(expr->op, left, right);
            return std::any_cast<double>(left) <= std::any_cast<double>(right);

        // arithmetics
        case PLUS:
            if (left.type() == typeid(double) && right.type() == typeid(double))
                return std::any_cast<double>(left) + std::any_cast<double>(right);

            if (left.type() == typeid(std::string) && right.type() == typeid(std::string))
                return std::any_cast<std::string>(left) + std::any_cast<std::string>(right);

            throw RuntimeError{expr->op, "Operands must be 2 numbers or 2 strings"};
        case MINUS:
            if (left.type() == typeid(double) && right.type() == typeid(double))
                return std::any_cast<double>(left) - std::any_cast<double>(right);
        case STAR:
            if (left.type() == typeid(double) && right.type() == typeid(double))
                return std::any_cast<double>(left) * std::any_cast<double>(right);
        case SLASH:
            if (left.type() == typeid(double) && right.type() == typeid(double))
                return std::any_cast<double>(left) / std::any_cast<double>(right);
    
        default: break;
    }

    return {}; // unreachable, here to make the compiler happy
}

std::any Interpreter::visitGroupingExpr(std::shared_ptr<GroupingExpr> expr)
{
    // evaluating parentheses
    return evaluate(expr->expression);
}

std::any Interpreter::visitLiteralExpr(std::shared_ptr<LiteralExpr> expr)
{
    // evaluating literals
    return expr->value;
}

std::any Interpreter::visitUnaryExpr(std::shared_ptr<UnaryExpr> expr)
{
    // evaluate unary expressions
    std::any right = evaluate(expr->right);

    switch (expr->op.type)
    {
        case BANG: return !is_truthy(right);
        case MINUS:
            check_num_operand(expr->op, right);
            return -std::any_cast<double>(right);
        
        default: break;
    }

    return {}; // unreachable, here to make the compiler happy
}

std::any Interpreter::visitVarExpr(std::shared_ptr<VarExpr> expr)
{
    std::any value = environment->get(expr->name);

    if (value.type() == typeid(nullptr))
        throw RuntimeError{expr->name, "Variable not initialized"};

    return value;
}

std::any Interpreter::visitLogicalExpr(std::shared_ptr<LogicalExpr> expr)
{
    std::any left = evaluate(expr->left);

    if (expr->op.type == OR)
    {
        if (is_truthy(left))
            return left;
    }
    else
    {
        if (!is_truthy(left))
            return left;
    }

    return evaluate(expr->right);
}

std::any Interpreter::visitCallExpr(std::shared_ptr<CallExpr> expr)
{
    std::any callee = evaluate(expr->callee);
    std::vector<std::any> arguments;

    for (const std::shared_ptr<Expr>& argument : expr->arguments)
        arguments.push_back(evaluate(argument));

    // pointers in a std::any wrapper must be unwrapped before they can be cast
    std::shared_ptr<NblCallable> function;

    // if (callee.type() == typeid(std::shared_ptr<NblFunction>))
    //     function = std::any_cast<std::shared_ptr<NblFunction>>(callee);
    // else
    //     throw RuntimeError(expr->paren, "Can only call functions and classes");
    if (callee.type() == typeid(NativeClock))
        function = std::any_cast<std::shared_ptr<NativeClock>>(callee);
    else
        throw RuntimeError(expr->paren, "Can only call functions");

    if (arguments.size() != function->arity())
        throw RuntimeError(expr->paren, "Expected " + std::to_string(function->arity()) + " arguments but got " + std::to_string(arguments.size()));

    return function->call(*this, std::move(arguments));
}


std::any Interpreter::evaluate(std::shared_ptr<Expr> expr)
{
    // sends expression back into interpreter's visitor implementation
    return expr->accept(*this);
}

void Interpreter::execute(std::shared_ptr<Stmt> stmt)
{
    stmt->accept(*this);
}

void Interpreter::execute_block(const std::vector<std::shared_ptr<Stmt>>& statements, std::shared_ptr<Environment> environment)
{
    std::shared_ptr<Environment> previous_env = this->environment;

    try
    {
        this->environment = environment;

        for (const std::shared_ptr<Stmt>& statement : statements)
            execute(statement);
    }
    catch (...)
    {
        this->environment = previous_env;
        throw;
    }

    this->environment = previous_env;
}

void Interpreter::check_num_operand(const Token& op, const std::any& operand)
{
    if (operand.type() == typeid(double))
        return;
    
    throw RuntimeError{op, "Operand must be a number"};
}

void Interpreter::check_num_operands(const Token& op, const std::any& left, const std::any& right)
{
    if (left.type() == typeid(double) && right.type() == typeid(double))
        return;
    
    throw RuntimeError{op, "Operands must be numbers"};
}

bool Interpreter::is_truthy(const std::any& obj)
{
    if (obj.type() == typeid(nullptr))
        return false;
    
    if (obj.type() == typeid(bool))
        return std::any_cast<bool>(obj);
    
    return true;
}

bool Interpreter::is_equal(const std::any& obj1, const std::any& obj2)
{
    if (obj1.type() == typeid(nullptr) && obj2.type() == typeid(nullptr))
        return true;

    if (obj1.type() == typeid(nullptr))
        return false;

    if (obj1.type() == typeid(std::string) && obj2.type() == typeid(std::string))
        return std::any_cast<std::string>(obj1) == std::any_cast<std::string>(obj2);

    if (obj1.type() == typeid(double) && obj2.type() == typeid(double))
        return std::any_cast<double>(obj1) == std::any_cast<double>(obj2);

    if (obj1.type() == typeid(bool) && obj2.type() == typeid(bool))
        return std::any_cast<bool>(obj1) == std::any_cast<bool>(obj2);

    return false;
}

std::string Interpreter::stringify(const std::any& obj)
{
    if (obj.type() == typeid(nullptr))
        return "nil";

    if (obj.type() == typeid(double))
    {
        std::string text = std::to_string(std::any_cast<double>(obj));

        if (text[text.length() - 2] == '.' && text[text.length() - 1] == '0')
            text = text.substr(0, text.length() - 2);

        return text;
    }

    if (obj.type() == typeid(std::string))
        return std::any_cast<std::string>(obj);

    if (obj.type() == typeid(bool))
        return std::any_cast<bool>(obj) ? "true" : "false";
    
    // if (obj.type() == typeid(std::shared_ptr<NblFunction>))
    //     return std::any_cast<std::shared_ptr<NblFunction>>(obj)->to_string();

    return "Error in stringify: Invalid object type";
}
