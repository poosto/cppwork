#include <coroutine>
#include <optional>
#include <print>
#include <thread>

struct Timer {
public:
  struct Awaitable {
    Timer &tmr;
    std::coroutine_handle<> handle;
    Awaitable *next_{};

    auto await_ready() const noexcept -> bool { return false; }

    auto await_suspend(std::coroutine_handle<> suspended) noexcept -> void {
      next_ = std::exchange(tmr.awaiter_, this);
      handle = suspended;
    }

    auto await_resume() const noexcept -> void {}
  };

  auto operator co_await() -> Awaitable { return Awaitable{*this}; }

  auto tick() -> void {
    // Store old list so that if something co_awaits the timer again after being
    // resumed, it isn't immediately invoked in the same tick iteration.
    // This will wipe the current awaiter_ list (nullptr), store it in this temp
    // variable, then if any new awaiters are added to awaiter_ it's to a fresh
    // list!
    auto *cur = std::exchange(awaiter_, nullptr);

    // Exchange awaiter_ ptr with nullptr, only resume if non-nullptr to begin
    // with. This ensures we don't resume the same awaiter twice
    while (cur) {
      auto *a = std::exchange(cur, cur->next_);
      a->handle.resume();
    }
  }

private:
  // TODO: make linked-list/vector
  Awaitable *awaiter_;
};

template <typename T> struct Task {
public:
  struct promise_type;

  Task(std::coroutine_handle<promise_type> coro) : coro_{coro} {}
  ~Task() { coro_.destroy(); }

  auto start() -> void { coro_.resume(); }

  auto result() const -> std::optional<T>
    requires(!std::is_void_v<T>)
  {
    return coro_.promise().result;
  }

  auto operator co_await() {
    struct awaiter {
      Task &t;

      auto await_ready() const noexcept -> bool { return false; }
      auto await_suspend(std::coroutine_handle<> suspended) noexcept -> void {
        t.coro_.promise().parent = suspended;
        t.coro_.resume();
      }
      auto await_resume() const noexcept {
        if constexpr (!std::is_void_v<T>) {
          return t.coro_.promise().result;
        }
      }
    };

    return awaiter{*this};
  }

private:
  std::coroutine_handle<promise_type> coro_;
};

template <typename T> struct return_handler {
  std::optional<T> result;
  auto return_value(T &&val) -> void { result.emplace(std::move(val)); }
};

template <> struct return_handler<void> {
  auto return_void() -> void {}
};

template <typename T> struct Task<T>::promise_type : return_handler<T> {
  std::coroutine_handle<> parent{};

  auto get_return_object() -> Task {
    return {std::coroutine_handle<promise_type>::from_promise(*this)};
  }

  auto unhandled_exception() { std::terminate(); }

  auto initial_suspend() noexcept -> std::suspend_always { return {}; }
  auto final_suspend() noexcept {
    struct final_awaiter {
      auto await_ready() const noexcept -> bool { return false; }

      auto await_suspend(std::coroutine_handle<promise_type> suspended) noexcept
          -> std::coroutine_handle<> {
        if (suspended.promise().parent) {
          // SYMMETRIC TRANSFER: does not create a new stack frame
          // It's a tail call so instead of nesting a bunch of stack frames,
          // reuse the same one! The coroutine/heap frame ofc lives until the
          // destructure is called (.destroy())
          return suspended.promise().parent;
        } else {
          return std::noop_coroutine();
        }
      }

      auto await_resume() const noexcept -> void {}
    };

    return final_awaiter{};
  }
};

auto foo(Timer &timer) -> Task<int> {
  std::println("Before suspend");
  co_await timer;
  std::println("After suspend");
  co_return 42;
}

auto chained(Timer &timer) -> Task<int> {
  std::println("Outer coroutine start");
  auto x = co_await foo(timer);
  std::println("Outer coroutine end: {}", x.value_or(-1));
  co_return 69;
}

auto bar(Timer &timer) -> Task<void> {
  std::println("Before first tick");
  co_await timer;
  std::println("Before second tick");
  co_await timer;
  std::println("After first + second tick");
  co_return;
}

int main() {
  auto timer = Timer{};

  auto t1 = chained(timer);
  auto t2 = bar(timer);

  t1.start();
  t2.start();

  // Empty ticks should do nothing
  constexpr size_t NUM_ITERATIONS = 4;
  for (int i = 0; i < NUM_ITERATIONS; ++i) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    timer.tick();
  }

  std::println("Result: {}", t1.result().value_or(-1));
}
