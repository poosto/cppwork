#include <iostream>
#include <string>
#include <typeindex>

template <char Key> struct typefmt {
  using type = void;
};

template <> struct typefmt<'d'> {
  using type = int;
};

template <> struct typefmt<'f'> {
  using type = double;
};

template <char Key> using typefmt_t = typefmt<Key>::type;

template <size_t N> struct CompileTimeString {
  char data[N];

  constexpr CompileTimeString(const char (&str)[N]) {
    for (size_t i = 0; i < N; ++i)
      data[i] = str[i];
  }

  template <size_t I> constexpr char &operator[](size_t) const {
    return data[I];
  }

  constexpr operator const char *() const { return data; }

  constexpr size_t size() const { return N; }
};

template <size_t N> constexpr size_t strlencp(const char (&)[N]) { return N; }

#define print(str, ...)                                                        \
  do {                                                                         \
    verify_impl<str, 0, strlencp(str), false>(__VA_ARGS__);                    \
    printf(str, __VA_ARGS__);                                                  \
  } while (0)

template <CompileTimeString S, size_t I, size_t N, bool PrevIsPercent,
          typename... Args>
constexpr void verify_impl(Args...) {
  return;
}

// template <CompileTimeString S, size_t I, size_t N, bool PrevIsPercent,
// typename T, typename... Args> constexpr void verify_impl(T arg, Args... rest)
// {
//     if (I >= N - 1) {
//         return;
//     }
//
//     constexpr char c = S[I];
//
//     if constexpr (PrevIsPercent) {
//         static_assert(std::is_same_v<typefmt_t<c>, T>);
//         verify_impl<S, I + 1, N, (c == '%')>(rest...);
//     } else {
//         verify_impl<S, I + 1, N, (c == '%')>(arg, rest...);
//     }
//
//     return;
// }

struct Foo {
  int *x_;

  constexpr Foo() : x_{new int{5}} {}
  constexpr ~Foo() { delete x_; } // don't delete x_;
};

int main() {
  // print("%f Hello %f\n", 42.0, 5);

  []() consteval -> void { constexpr Foo x; }();

  return 0;
}
