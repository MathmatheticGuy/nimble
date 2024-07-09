#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP

#include <memory>
#include <any>
#include <string>
#include <map>
#include <functional>
#include <utility>

#include "error.hpp"
#include "token.hpp"

class Environment : public std::enable_shared_from_this<Environment>
{
    std::shared_ptr<Environment> enclosing;
    std::map<std::string, std::any> values;

    public:
        Environment();
        Environment(std::shared_ptr<Environment> enclosing);

        std::any get(const Token& name);
        void assign(const Token& name, std::any value);
        void define(const std::string& name, std::any value);
};

#endif
