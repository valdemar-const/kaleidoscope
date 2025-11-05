#pragma once

#include <vector>
#include <string>
#include <optional>
#include <functional>
#include <bit>
#include <typeindex>
#include <type_traits>

#include <boost/uuid.hpp>
#include <boost/callable_traits.hpp>

#include <cinttypes>
#include <cstddef>

namespace kaleidoscope::type::memory
{

constexpr size_t
round_to_power_of_two(size_t value)
{
    if (value == 0)
    {
        return 1;
    }
    return std::bit_ceil(value);
}

struct Layout
{
    virtual ~Layout(void) = default;

    virtual size_t size(void) const      = 0;
    virtual size_t alignment(void) const = 0;
};

struct Primitive final : public Layout
{
    ~Primitive(void) override = default;

    Primitive(void)
        : size_(sizeof(std::byte))
        , alignment_(alignof(std::byte))
    {
    }

    Primitive(size_t size, size_t alignment)
        : size_(size)
        , alignment_(round_to_power_of_two(alignment))
    {
    }

    Primitive(const Primitive &)            = default;
    Primitive(Primitive &&)                 = default;
    Primitive &operator=(const Primitive &) = default;
    Primitive &operator=(Primitive &&)      = default;

  public:

    size_t
    size(void) const override
    {
        return size_;
    }

    size_t
    alignment(void) const override
    {
        return alignment_;
    }

  protected:

    size_t size_;
    size_t alignment_;
};

struct Array final : public Layout
{
    ~Array(void) override = default;

    Array(Layout *element, size_t size = 1)
        : element_(element)
        , size_(size)
    {
    }

    Array(const Array &)            = default;
    Array(Array &&)                 = default;
    Array &operator=(const Array &) = default;
    Array &operator=(Array &&)      = default;

  public:

    size_t
    size(void) const override
    {
        return (element_->size() * size_ + alignment() - 1) / alignment() * alignment();
    }

    size_t
    alignment(void) const override
    {
        return element_->alignment();
    }

  protected:

    Layout *element_;
    size_t  size_;
};

#if 0
struct Product final : public Layout
{
    ~Product(void) override = default;

    Product(void) = default;

    Product(std::initializer_list<Layout *> fields)
        : fields_(std::move(fields))
    {
    }

    Product(const Product &)            = default;
    Product(Product &&)                 = default;
    Product &operator=(const Product &) = default;
    Product &operator=(Product &&)      = default;

  public:

    Product &
    add_field(Layout *field)
    {
        fields_.emplace_back(field);

        return *this;
    }

  public:

    size_t
    size(void) override
    {
        if (fields_.empty())
        {
            return empty_layout().size();
        }

        return size_; // TODO: calculate from fields
    }

    size_t
    alignment(void) override
    {
        if (fields_.empty())
        {
            return empty_layout().alignment();
        }

        return alignment_; // TODO: calculate from fields
    }

  protected:

    std::vector<Layout *> fields_;

  private:

    static Primitive &
    empty_layout(void)
    {
        static Primitive empty_layout_ {sizeof(std::byte), alignof(std::byte)};
        return empty_layout_;
    }
};

struct Sum final : public Layout
{
    ~Sum(void) override = default;

    Sum(void) = default;

    Sum(std::initializer_list<Layout *> fields)
        : fields_(std::move(fields))
    {
    }

    Sum(const Sum &)            = default;
    Sum(Sum &&)                 = default;
    Sum &operator=(const Sum &) = default;
    Sum &operator=(Sum &&)      = default;

  public:

    Sum &
    add_field(Layout *field)
    {
        fields_.emplace_back(field);

        return *this;
    }

  public:

    size_t
    size(void) override
    {
        if (fields_.empty())
        {
            return empty_layout().size();
        }

        return size_; // TODO: calculate from fields
    }

    size_t
    alignment(void) override
    {
        if (fields_.empty())
        {
            return empty_layout().alignment();
        }

        return alignment_; // TODO: calculate from fields
    }

  protected:

    std::vector<Layout *> fields_;

  private:

    static Primitive &
    empty_layout(void)
    {
        static Primitive empty_layout_ {sizeof(std::byte), alignof(std::byte)};
        return empty_layout_;
    }
};
#endif

} // namespace kaleidoscope::type::memory

namespace kaleidoscope
{

namespace traits
{
    template<typename T>
    concept Type_Basic_Scalar = std::is_fundamental_v<T>
                             && std::is_scalar_v<T>
                             && !std::is_pointer_v<T>
                             && !std::is_array_v<T>;

    template<typename T>
    concept Type_Callable = std::is_invocable_v<T>;

} // namespace traits
} // namespace kaleidoscope

namespace kaleidoscope::type
{

struct Descriptor
{
    virtual ~Descriptor(void) = default;

    virtual std::optional<std::string_view>
    type_name(void) const
    {
        return type_name_;
    }

  protected:

    std::optional<std::string_view> type_name_;
};

struct Base_Type final : public Descriptor
{
    virtual ~Base_Type(void) = default;

    size_t  byte_size;
    size_t  bit_size;
    uint8_t is_signed   : 1;
    uint8_t is_floating : 1;
};

struct Array final : public Descriptor
{
    virtual ~Array(void) = default;

    Descriptor *element_type;
    size_t      size;
};

struct Pointer final : public Descriptor
{
    virtual ~Pointer(void) = default;

    Descriptor *parent;
};

struct Reference final : public Descriptor
{
    virtual ~Reference(void) = default;

    Descriptor *parent;
};

struct Mutable_Qualifier final : public Descriptor
{
    Descriptor *parent;
};

struct Function_Definition final : public Descriptor
{
    virtual ~Function_Definition(void) = default;

    Descriptor                                            *result;
    std::vector<std::pair<std::string_view, Descriptor *>> args;
};

struct Info
{
    struct Impl
    {
        std::function<void(void *self)>              initialize;
        std::function<void(void *self, void *other)> clone;
        std::function<void(void *self, void *other)> move;
        std::function<void(void *self)>              dispose;
    };

    Info(std::string name, std::unique_ptr<memory::Layout> layout, Impl impl)
        : name_(std::move(name))
        , layout_(std::move(layout))
        , impl_(std::move(impl))
    {
    }

    Info(const Info &)            = delete;
    Info(Info &&)                 = default;
    Info &operator=(const Info &) = delete;
    Info &operator=(Info &&)      = default;

  public:

    bool
    operator==(const Info &other)
    {
        if (this == &other)
        {
            return true;
        }

        return name_ == other.name_; // FIXME: наивно, но если имена типов уникальны - сработает.
    }

  public:

    std::string_view
    name() const
    {
        return {name_};
    }

  protected:

    std::string                     name_;
    std::unique_ptr<memory::Layout> layout_;
    Impl                            impl_;
};

struct CallableSign
{
    Info              result_t;
    size_t            min_arity = 0;
    size_t            max_arity = 0;
    std::vector<Info> args;
};

template<traits::Type_Basic_Scalar T>
Info
make_info(std::string name)
{
    auto layout = std::make_unique<memory::Primitive>(sizeof(T), alignof(T));

    const Info::Impl impl {
            .initialize = [size = layout->size()](void *self) -> void
            {
                std::memset(self, 0, size);
            },
            .clone = [size = layout->size()](void *self, void *other) -> void
            {
                std::memcpy(self, other, size);
            },
            .move = [size = layout->size()](void *self, void *other) -> void
            {
                std::memcpy(self, other, size);
            },
            .dispose = [](void *self) -> void
            {
                return;
            },
    };

    return {
            std::move(name),
            std::move(layout),
            std::move(impl)
    };
}

template<traits::Type_Callable T>
CallableSign
make_callable_sign(std::string name)
{
    using Args           = boost::callable_traits::args_t<T>;
    using Result         = boost::callable_traits::return_type_t<T>;
    constexpr auto arity = std::tuple_size_v<Args>;

    static constexpr auto get_args_signatures = []<typename Tuple>(void) -> const std::vector<std::type_index> &
    {
        // for type in Tuple do args_type_signatures.emplace_back(typeid(type)); done
        static const auto result = []<size_t... I>(std::index_sequence<I...>) -> std::vector<Info>
        {
            std::vector<Info> arg_types;
            (arg_types.emplace_back(make_info(std::tuple_element_t<I, Tuple>)), ...);
            return arg_types;
        }(std::make_index_sequence<std::tuple_size_v<Tuple>> {});
        return result;
    };

    auto args_type_signatures = get_args_signatures.operator()<Args>();

    auto layout = std::make_unique<memory::Primitive>(sizeof(T), alignof(T));

    return {
            make_info<Result>(name),
            arity,
            arity,
            std::move(get_args_signatures)
    };
}

} // namespace kaleidoscope::type
