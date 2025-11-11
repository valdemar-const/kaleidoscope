#pragma once

#include <compiler/demangle.hpp>

#include <vector>
#include <string>
#include <optional>
#include <functional>
#include <bit>
#include <typeindex>
#include <type_traits>
#include <any>
#include <ranges>
#include <numeric>
#include <algorithm>

#include <boost/uuid.hpp>
#include <boost/callable_traits.hpp>
#include <boost/functional/hash.hpp>

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
    concept Type_Callable =
            std::is_function_v<T> || std::is_invocable_v<T>
            || requires(T t) { &std::decay_t<T>::operator(); };
} // namespace traits
} // namespace kaleidoscope

namespace kaleidoscope::type
{

enum class Encoding : uint8_t
{
    Signed,   // twos_complement
    Unsigned, // binary
    Float,    // IEEE754
    Boolean   // 0/1
};

struct Operator_Properties
{
    using Precedence = size_t;

    enum class Associativity : uint8_t
    {
        Left,
        Right
    };

    enum class Kind : uint8_t
    {
        Postfix,
        Prefix,
        Infix
    };

    Kind          kind          = Kind::Infix;
    Associativity associativity = Associativity::Left;
    Precedence    precedence    = 0; /**< lesser is higher */
};

struct Info
{
    virtual ~Info(void)                                   = default;
    virtual size_t                fingerprint(void) const = 0;
    virtual std::unique_ptr<Info> clone(void) const       = 0;

  public:

    // Для STL контейнеров
    friend bool
    operator==(const Info &a, const Info &b)
    {
        return &a == &b || a.fingerprint() == b.fingerprint();
    }

    friend bool
    operator<(const Info &a, const Info &b)
    {
        return a.fingerprint() < b.fingerprint();
    }

    struct Hash
    {
        size_t
        operator()(const Info &type) const
        {
            return type.fingerprint();
        }

        size_t
        operator()(const std::unique_ptr<Info> &type) const
        {
            return type->fingerprint();
        }
    };

    std::optional<std::string> name;
};

struct Void final : public Info
{
    ~Void(void) override = default;

    std::unique_ptr<Info>
    clone(void) const override
    {
        return std::make_unique<Void>(*this);
    }

    size_t
    fingerprint(void) const override
    {
        size_t seed = 0;
        boost::hash_combine(seed, "void");
        if (name)
        {
            boost::hash_combine(seed, *name);
        }
        return seed;
    }
};

struct Primitive final : public Info
{
    ~Primitive(void) override = default;

    std::unique_ptr<Info>
    clone(void) const override
    {
        return std::make_unique<Primitive>(*this);
    }

    size_t
    fingerprint(void) const override
    {
        size_t seed = 0;
        boost::hash_combine(seed, "primitive");
        if (name)
        {
            boost::hash_combine(seed, *name);
        }
        boost::hash_combine(seed, static_cast<size_t>(encoding));
        boost::hash_combine(seed, byte_size);
        return seed;
    }

    size_t   byte_size = sizeof(std::byte);
    size_t   alignment = alignof(std::byte);
    Encoding encoding  = Encoding::Signed;
};

struct Formal_Parameter final : public Info
{
    ~Formal_Parameter(void) override = default;

    std::unique_ptr<Info>
    clone(void) const override
    {
        return std::make_unique<Formal_Parameter>(name, type->clone());
    }

    size_t
    fingerprint(void) const override
    {
        size_t seed = 0;
        boost::hash_combine(seed, "formal_parameter");
        if (name)
        {
            boost::hash_combine(seed, *name);
        }
        boost::hash_combine(seed, type->fingerprint());
        return seed;
    }

    std::unique_ptr<Info> type;
};

struct Function final : public Info
{
    using Args               = std::vector<std::unique_ptr<Formal_Parameter>>;
    ~Function(void) override = default;

    std::unique_ptr<Info>
    clone(void) const override
    {
        auto cloned_args = std::accumulate(
                arguments.cbegin(), arguments.cend(), Args {}, [](auto acc, const auto &arg)
                {
                    acc.emplace_back(arg->clone());
                    return acc;
                }
        );
        return std::make_unique<Function>(name, ret_type->clone(), std::move(cloned_args));
    }

    size_t
    fingerprint(void) const override
    {
        size_t seed = 0;
        boost::hash_combine(seed, "fn");
        if (name)
        {
            boost::hash_combine(seed, *name);
        }
        boost::hash_combine(seed, ret_type->fingerprint());
        for (const auto &arg : arguments)
        {
            boost::hash_combine(seed, arg->fingerprint());
        }
        return seed;
    }

    std::unique_ptr<Info>                          ret_type;
    std::vector<std::unique_ptr<Formal_Parameter>> arguments;
};

struct Variable final : public Info
{
    std::unique_ptr<Info>
    clone(void) const override
    {
        return std::make_unique<Primitive>(name, type->clone());
    }

    size_t
    fingerprint(void) const override
    {
        size_t seed = 0;
        boost::hash_combine(seed, "var");
        if (name)
        {
            boost::hash_combine(seed, *name);
        }
        boost::hash_combine(seed, type->fingerprint());

        return seed;
    }

    std::unique_ptr<Info> type;
};

struct Mutable_Qualifier final : public Info
{
    std::unique_ptr<Info>
    clone(void) const override
    {
        return std::make_unique<Primitive>(name, redirect->clone());
    }

    size_t
    fingerprint(void) const override
    {
        size_t seed = 0;
        boost::hash_combine(seed, "const");
        boost::hash_combine(seed, redirect->fingerprint());

        return seed;
    }

    std::unique_ptr<Info> redirect;
};

struct Operator final : public Info
{
    ~Operator(void) override = default;

    std::unique_ptr<Info>
    clone(void) const override
    {
        return std::make_unique<Operator>(name, prototype->clone(), properties);
    }

    size_t
    fingerprint(void) const override
    {
        size_t seed = 0;
        boost::hash_combine(seed, "operator");
        if (name)
        {
            boost::hash_combine(seed, *name);
        }
        boost::hash_combine(seed, prototype->fingerprint());
        boost::hash_combine(seed, properties.kind);
        boost::hash_combine(seed, properties.associativity);
        boost::hash_combine(seed, properties.precedence);

        return seed;
    }

    std::unique_ptr<Function> prototype;
    Operator_Properties       properties;
};

struct Cpp_Type final : public Info
{
    ~Cpp_Type(void) override = default;

    std::unique_ptr<Info>
    clone(void) const override
    {
        return std::make_unique<Operator>(name, external, inner->clone());
    }

    size_t
    fingerprint(void) const override
    {
        size_t seed = 0;
        boost::hash_combine(seed, "externcpp");
        boost::hash_combine(seed, name.value());
        boost::hash_combine(seed, external);
        boost::hash_combine(seed, inner->fingerprint());
        boost::hash_combine(seed, 0);
        return seed;
    }

    std::type_index       external;
    std::unique_ptr<Info> inner;
};

template<traits::Type_Basic_Scalar T>
consteval Encoding
encoding_of()
{
    constexpr bool is_floating = std::is_floating_point_v<T>;
    constexpr bool is_signed   = std::is_signed_v<T>;
    constexpr bool is_unsigned = std::is_unsigned_v<T>;
    constexpr bool is_boolean  = std::is_same_v<std::decay_t<T>, bool>;

    if constexpr (is_floating)
    {
        return Encoding::Float;
    }
    else if constexpr (is_boolean)
    {
        return Encoding::Boolean;
    }
    else if constexpr (is_signed)
    {
        return Encoding::Signed;
    }
    else if constexpr (is_unsigned)
    {
        return Encoding::Unsigned;
    }
    else
    {
        static_assert(false, "Unsupported type Encoding");
    }
}

template<traits::Type_Basic_Scalar T>
std::unique_ptr<Info>
make_info(std::string name)
{
    auto info      = std::make_unique<Primitive>(compiler::demangle(name.data()), sizeof(T), alignof(T), encoding_of<T>());
    auto externcpp = std::make_unique<Cpp_Type>(typeid(T), std::move(info));

    return externcpp;
}

} // namespace kaleidoscope::type

namespace std
{
// Специализация std::hash
template<>
struct hash<kaleidoscope::type::Info>
{
    size_t
    operator()(const kaleidoscope::type::Info &type) const
    {
        return type.fingerprint();
    }
};
} // namespace std
