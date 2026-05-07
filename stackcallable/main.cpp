#include <new>
#include <print>
#include <type_traits>
#include <utility>

template <size_t Capacity, class> class StackCallable;

template <size_t Capacity, class R, class... Args>
class StackCallable<Capacity, R(Args...)> {
public:
  template <class F>
    requires(!std::is_same_v<std::remove_cvref_t<F>, StackCallable> &&
             std::is_nothrow_invocable_r_v<R, F, Args...> &&
             std::is_move_constructible_v<F> &&
             std::is_nothrow_move_constructible_v<F> &&
             std::is_copy_constructible_v<F> &&
             (std::is_default_constructible_v<R> || std::is_void_v<R>) &&
             sizeof(F) <= Capacity)
  StackCallable(F &&callable) {
    // std::byte[] has implicit cast to void*
    new (callable_) F{std::move(callable)};

    // Lambdas with no capture can be stored in function pointers, since they
    // have static storage like regular functions. If there was a capture here,
    // the lambda would be a functor and this wouldn't compile
    invoke_ = [](void *ptr, Args... args) -> R {
      return launder_cast<F>(ptr)->operator()(std::forward<Args>(args)...);
    };

    if constexpr (!std::is_trivially_destructible_v<F>) {
      destroy_ = [](void *ptr) -> void { launder_cast<F>(ptr)->~F(); };
    }

    copy_ = [](void *dst, const void *src) -> void {
      new (dst) F{*launder_cast<F>(src)};
    };

    move_ = [](void *dst, void *src) noexcept -> void {
      new (dst) F{std::move(*launder_cast<F>(src))};
    };
  }

  ~StackCallable() {
    if (destroy_) {
      destroy_(callable_);
    }
  }

  StackCallable(const StackCallable &other)
      : invoke_{other.invoke_}, destroy_{other.destroy_}, copy_{other.copy_},
        move_{other.move_} {
    copy_(callable_, other.callable_);
  }

  auto operator=(const StackCallable &other) -> StackCallable & {
    if (this != &other) {
      StackCallable<Capacity, R(Args...)> temp{other};
      swap(temp);
    }

    return *this;
  }

  StackCallable(StackCallable &&other) noexcept
      : invoke_{std::exchange(other.invoke_, nullptr)},
        destroy_{std::exchange(other.destroy_, nullptr)},
        copy_{std::exchange(other.copy_, nullptr)},
        move_{std::exchange(other.move_, nullptr)} {
    move_(callable_, other.callable_);
  }

  auto operator=(StackCallable &&other) noexcept -> StackCallable & {
    if (this != &other) {
      StackCallable<Capacity, R(Args...)> temp{std::move(other)};
      swap(temp);
    }

    return *this;
  }

  // TODO: const operator() is unsupported — breaks with mutable lambdas.
  // Proper fix: add a R(Args...) const specialization (like
  // std::move_only_function), or do some weird C++23 deducing this:
  // https://devblogs.microsoft.com/cppblog/cpp23-deducing-this/
  auto operator()(Args... args) noexcept -> R {
    if constexpr (std::is_void_v<R>) {
      if (invoke_)
        invoke_(callable_, args...);
    } else {
      return invoke_ ? invoke_(callable_, args...) : R{};
    }
  }

private:
  // Max alignment to avoid UB after placement-new
  // Can't align by F due to type-erasure
  alignas(std::max_align_t) std::byte callable_[Capacity];

  auto (*invoke_)(void *, Args...) -> R{};
  auto (*destroy_)(void *) -> void{};
  auto (*copy_)(void *, const void *) -> void{};
  auto (*move_)(void *, void *) noexcept -> void{};

  auto swap(StackCallable<Capacity, R(Args...)> &other) -> void {
    std::swap(callable_, other.callable_);
    std::swap(invoke_, other.invoke_);
    std::swap(destroy_, other.destroy_);
    std::swap(copy_, other.copy_);
    std::swap(move_, other.move_);
  }

  template <typename T> static auto launder_cast(void *ptr) -> T * {
    return std::launder(reinterpret_cast<T *>(ptr));
  }

  template <typename T> static auto launder_cast(const void *ptr) -> const T * {
    return std::launder(reinterpret_cast<const T *>(ptr));
  }
};

int main() {
  std::println("Hello world");

  StackCallable<16, void()> foo{[]() noexcept { std::println("test!"); }};
  StackCallable<16, void()> bar{foo};
  StackCallable<16, void()> baz{std::move(bar)};
  foo();
  baz();

  int x = 69;
  StackCallable<16, float(double)> capture{
      [&](double y) noexcept -> float { return x + y + 420; }};
  StackCallable<16, float(double)> capture_moved{std::move(capture)};
  std::println("Result: {:.2f}", capture_moved(13.37));

  StackCallable<8, void()> mutablefoo{[=]() mutable noexcept {
    std::println("test, mutable!");
    ++x;
  }};
  mutablefoo();
  std::println("Value of x (should be same due to capture-by-value): {}", x);

  return 0;
}
