#ifndef NBL_LIST_HPP
#define NBL_LIST_HPP

#pragma once
#include <vector>
#include <any>

#include "expr.hpp"

struct ListType
{
    std::vector<std::any> elements;

    void append(std::any value);
    std::any get_element_at(int index);
    int get_length();
};

#endif
