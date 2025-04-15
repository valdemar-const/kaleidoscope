#pragma once

#include <kaleidoscope/state.hpp>
#include <kaleidoscope/ast/visitor.hpp>

namespace kaleidoscope
{

struct precedence : ast::utils::Visitor_Node_CRTP<precedence, kaleidoscope::ast::Node>
{
    using Operator_Info     = kaleidoscope::state::operator_properties;
    using Bin_Op_Precedence = std::unordered_map<std::string, Operator_Info>;

  public:

    precedence(const Bin_Op_Precedence &precedence_);
    ~precedence(void) = default;

  protected:

    std::reference_wrapper<const Bin_Op_Precedence> precedence_;
};

} // namespace kaleidoscope
