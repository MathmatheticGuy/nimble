#include "expr.hpp"

AssignExpr::AssignExpr(Token name, std::shared_ptr<Expr> value)
    : name(std::move(name)), value(std::move(value)) {}

std::any AssignExpr::accept(ExprVisitor& visitor)
{
    return visitor.visitAssignExpr(shared_from_this());
}


BinaryExpr::BinaryExpr(std::shared_ptr<Expr> left, Token op, std::shared_ptr<Expr> right)
    : left(std::move(left)), op(std::move(op)), right(std::move(right)) {}

std::any BinaryExpr::accept(ExprVisitor& visitor)
{
    return visitor.visitBinaryExpr(shared_from_this());
}


GroupingExpr::GroupingExpr(std::shared_ptr<Expr> expression)
    : expression(std::move(expression)) {}

std::any GroupingExpr::accept(ExprVisitor& visitor)
{
    return visitor.visitGroupingExpr(shared_from_this());
}


LiteralExpr::LiteralExpr(std::any value)
    : value(std::move(value)) {}

std::any LiteralExpr::accept(ExprVisitor& visitor)
{
    return visitor.visitLiteralExpr(shared_from_this());
}


UnaryExpr::UnaryExpr(Token op, std::shared_ptr<Expr> right)
    : op(std::move(op)), right(std::move(right)) {}

std::any UnaryExpr::accept(ExprVisitor& visitor)
{
    return visitor.visitUnaryExpr(shared_from_this());
}

VarExpr::VarExpr(Token name)
    : name(std::move(name)) {}

std::any VarExpr::accept(ExprVisitor& visitor)
{
    return visitor.visitVarExpr(shared_from_this());
}


LogicalExpr::LogicalExpr(std::shared_ptr<Expr> left, Token op, std::shared_ptr<Expr> right)
    : left(std::move(left)), op(std::move(op)), right(std::move(right)) {}

std::any LogicalExpr::accept(ExprVisitor& visitor)
{
    return visitor.visitLogicalExpr(shared_from_this());
}

CallExpr::CallExpr(std::shared_ptr<Expr> callee, Token paren, std::vector<std::shared_ptr<Expr>> arguments)
    : callee{std::move(callee)}, paren{std::move(paren)}, arguments{std::move(arguments)} {}

std::any CallExpr::accept(ExprVisitor& visitor)
{
    return visitor.visitCallExpr(shared_from_this());
}


FunctionExpr::FunctionExpr(std::vector<Token> parameters, std::vector<std::shared_ptr<Stmt>> body)
    : parameters(std::move(parameters)), body(std::move(body)) {}

std::any FunctionExpr::accept(ExprVisitor& visitor)
{
    return visitor.visitFunctionExpr(shared_from_this());
}
