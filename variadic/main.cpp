#include <iostream>
#include <type_traits>
#include <functional>

using std::cout, std::endl;

/*template <typename T>
T sum(T first) {
    return first;
}

template <typename T, typename... Args>
requires (std::is_arithmetic_v<T> && ... && std::is_arithmetic_v<Args>)
auto sum(T first, Args... rest) {
    return first + sum(rest...);
}*/


template <bool Cond>
struct hi_enable {};

template <>
struct hi_enable<true> { using type = void; };

template <bool Cond>
using hi_enable_t = hi_enable<Cond>::type;

template <typename... Args>
requires (std::is_arithmetic_v<Args> && ...)
auto sum(Args&&... args) {
    return (std::forward<Args>(args) + ...);
}

struct bad {
    int x;
};

template <typename T, typename = hi_enable_t<std::is_arithmetic_v<T>>>
int foo() {
    return 42;
}

template <typename T, typename = decltype(std::declval<T>() + std::declval<T>())>
int bar() {
    return 69;
}

template <typename... Args>
class MyTuple;

template <>
class MyTuple<> {};

template <typename T, typename... Args>
class MyTuple<T, Args...> {
public:
    MyTuple(T&& data, Args&&... args) : data_{std::forward<T>(data)}, next_{std::forward<Args>(args)...} {}

    ~MyTuple() = default;
    MyTuple(const MyTuple& other) = delete;
    MyTuple(MyTuple&& other) = delete;

    template <size_t N>
    auto get() {
        if constexpr (N == 0) {
            return data_;
        } else {
            return next_.template get<N-1>();
        }
    }

private:
    T data_;
    MyTuple<Args...> next_;
};

// Forward declaration
template <typename... Args>
class PTuple;

template <typename Head, typename... Tail>
class PTuple<Head, Tail...> : public PTuple<Tail...> {
public:
    using Base = PTuple<Tail...>;

    PTuple(Head data, Tail... next) :
        data_{data},
        Base{next...}
    {}

    Head get() const {
        return data_;
    }

private:
    Head data_;
};

template <>
class PTuple<> {};

template <size_t N, typename Tuple>
struct PTupleGet {
    static auto apply(const Tuple& tuple) {
        return PTupleGet<N-1, typename Tuple::Base>::apply(tuple);
    }
};

template<typename Tuple>
struct PTupleGet<0, Tuple> {
    static auto apply(const Tuple& tuple) {
        return tuple.get();
    }
};

template <typename Expected, typename Unexpected>
class Expect {
public:
    using value_type = Expected;
    using reference = Expected&;
    using const_reference = const Expected&;
    
    constexpr Expect(const Expected& val) : 
        has_value_{true},
        expected_val_{val}
    {}

    constexpr Expect(Expected&& val) : 
        has_value_{true},
        expected_val_{std::move(val)}
    {}

    constexpr Expect(const Unexpected& val) :
        has_value_{false},
        unexpected_val_{val}
    {}

    constexpr Expect(Unexpected&& val) :
        has_value_{false},
        unexpected_val_{std::move(val)}
    {}

    constexpr ~Expect() {
        if (has_value_) {
            if constexpr (!std::is_trivially_destructible_v<Expected>)
                expected_val_.~Expected();
        } else {
            if constexpr (!std::is_trivially_destructible_v<Unexpected>)
                unexpected_val_.~Unexpected();
        }
    }

    constexpr Expect(const Expect& other) :
        has_value_{other.has_value_}
    { 
        if (has_value_) {
            new (&expected_val_) Expected{other.expected_val_};
        } else {
            new (&unexpected_val_) Unexpected{other.unexpected_val_};
        }
    } 

    constexpr Expect(Expect&& other) noexcept :
        has_value_{other.has_value_}
    {
        if (has_value_) {
            new (&expected_val_) Expected{std::move(other.expected_val_)};
        } else {
            new (&unexpected_val_) Unexpected{std::move(other.unexpected_val_)};
        }
    }

    constexpr Expect& operator=(Expect other) {
        swap(other);
        return *this;
    }

    constexpr bool has_value() const {
        return has_value_;
    }

    constexpr reference operator*() {
        return expected_val_;
    }

    constexpr const_reference operator*() const {
        return expected_val_;
    }

private:
    bool has_value_{};
    union {
        Expected expected_val_;
        Unexpected unexpected_val_;
    };

    constexpr void swap(Expect& other) {
        auto swapper = [&](
            Expected& old_e, Unexpected& old_u,
            auto* new_e, auto* new_u
        ) {
            auto e_temp{std::move(old_e)};
            auto u_temp{std::move(old_u)};

            old_e.~Expected();
            old_u.~Unexpected();

            new (new_e) Expected{std::move(e_temp)};
            new (new_u) Unexpected{std::move(u_temp)};
            
            std::swap(has_value_, other.has_value_);
        };

        if (has_value_ && other.has_value_) {
            std::swap(expected_val_, other.expected_val_);
        } else if (!has_value_ && !other.has_value_) {
            std::swap(unexpected_val_, other.unexpected_val_);
        } else if (has_value_ && !other.has_value_) {
            swapper(expected_val_, other.unexpected_val_, &other.expected_val_, &unexpected_val_);
        } else {
            swapper(other.expected_val_, unexpected_val_, &expected_val_, &other.unexpected_val_);
        }
    }
};

template <typename T>
auto get_value(T t) {
    if constexpr (std::is_pointer_v<T>)
        return *t; // deduces return type to int for T = int*
    else
        return t;  // deduces return type to int for T = int
}

int main() {
    get_value(45);
    // bar<int>();
    // cout << sum(6, 2, 2.5) << endl;
    // cout << foo<int>() << endl;
    // cout << bar<bad>() << endl;

    MyTuple<int, char> x{4, 'a'};
    cout << x.get<0>() << x.get<1>() << endl;

    // using PT = PTuple<int, char, std::string>;
    // PT y{4, 'a', "hello"};
    // cout << PTupleGet<0, PT>::apply(y) << PTupleGet<1, PT>::apply(y) << endl;

    std::unique_ptr<int> myUnique = std::make_unique<int>(67);

    Expect<std::unique_ptr<int>, std::string> bob{std::move(myUnique)};
    cout << **bob << endl;

    Expect<std::unique_ptr<int>, std::string> fool{std::move(bob)};
    cout << fool.has_value() << " " << **fool << " " << *bob << endl;

    bob = std::move(fool);
    cout << fool.has_value() << " " << *fool << " " << **bob << endl;

    return 0;
}