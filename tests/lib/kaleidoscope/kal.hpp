#pragma once

#include <entt/entt.hpp>

#include <anyany/anyany.hpp>
#include <anyany/anyany_macro.hpp>

#include <boost/uuid.hpp>
#include <boost/uuid/random_generator.hpp>

#include <string_view>

namespace kaleidoscope
{
struct Ast; // forward decl
} // namespace kaleidoscope

namespace kaleidoscope::interface::method
{
anyany_method(get_uuid, (&self) requires(self.get_id())->boost::uuids::uuid);
anyany_method(value, (&self) requires(self.value())->uint8_t);
}; // namespace kaleidoscope::interface::method

namespace kaleidoscope::interface
{
using IGlobalEntity    = aa::any_with<method::get_uuid>;
using IGlobalEntityPtr = aa::poly_ptr<method::get_uuid>;
using IGlobalEntityRef = aa::poly_ref<method::get_uuid>;
} // namespace kaleidoscope::interface

namespace kaleidoscope::ecs::component
{
using uuid = boost::uuids::uuid;

struct LexemeNumericConstantU8
{
    LexemeNumericConstantU8(uint8_t value)
        : value_(value)
    {
    }

    LexemeNumericConstantU8(const LexemeNumericConstantU8 &)            = default;
    LexemeNumericConstantU8(LexemeNumericConstantU8 &&)                 = default;
    LexemeNumericConstantU8 &operator=(const LexemeNumericConstantU8 &) = default;
    LexemeNumericConstantU8 &operator=(LexemeNumericConstantU8 &&)      = default;

    operator uint8_t(void)
    {
        return value_;
    }

  protected:

    uint8_t value_;
};

} // namespace kaleidoscope::ecs::component

namespace kaleidoscope::ecs::adapter::ast
{

struct LexemeNumericConstantU8
{
    LexemeNumericConstantU8(entt::registry *owner, entt::entity id)
        : owner_(owner)
        , id_(id)
    {
    }

    ecs::component::uuid
    get_id(void)
    {
        return owner_->get<ecs::component::uuid>(id_);
    }

    uint8_t
    value(void)
    {
        return owner_->get<ecs::component::LexemeNumericConstantU8>(id_);
    }

  protected:

    entt::entity    id_;
    entt::registry *owner_;
};

}; // namespace kaleidoscope::ecs::adapter::ast

namespace kaleidoscope
{
using namespace interface;

struct context
{
    IGlobalEntity
    create_const_u8(uint8_t value)
    {
        auto node = reg_.create();
        reg_.emplace<ecs::component::uuid>(node, gen_uuid());
        reg_.emplace<ecs::component::LexemeNumericConstantU8>(node, value);
        return ecs::adapter::ast::LexemeNumericConstantU8(&reg_, node);
    }

  protected:

    entt::registry                         reg_;
    boost::uuids::random_generator_mt19937 gen_uuid;
};

}; // namespace kaleidoscope
