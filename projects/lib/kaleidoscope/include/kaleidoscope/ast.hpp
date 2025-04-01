#pragma once

#include <string>

namespace kaleidoscope::ast
{
struct Node
{
    virtual ~Node(void) = default;
};

template<typename T>
struct NodeCRTP : public Node
{
    ~NodeCRTP(void) override = default;
};

struct Function_Declaration : public NodeCRTP<Function_Declaration>
{
    ~Function_Declaration(void) override = default;
};

struct Function_Defenition : public NodeCRTP<Function_Defenition>
{
    ~Function_Defenition(void) override = default;
};

struct Functional_Call : public NodeCRTP<Functional_Call>
{
};

} // namespace kaleidoscope::ast

namespace kaleidoscope
{

struct Ast
{
    std::string source;
};

} // namespace kaleidoscope
