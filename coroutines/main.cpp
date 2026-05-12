#include <coroutine>
#include <optional>
#include <print>
#include <thread>

struct Timer {
public:
  struct Awaitable {
    Timer &tmr;
    std::coroutine_handle<> handle;

    bool await_ready() const noexcept { return false; }

    void await_suspend(std::coroutine_handle<> suspended) noexcept {
      tmr.awaiter_ = this;
      handle = suspended;
    }

    void await_resume() const noexcept {}
  };

  Awaitable operator co_await() { return Awaitable{*this}; }

  void tick() {
    // Exchange awaiter_ ptr with nullptr, only resume if non-nullptr to begin
    // with. This ensures we don't resume the same awaiter twice
    if (auto *a = std::exchange(awaiter_, nullptr)) {
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

  void start() { coro_.resume(); }

  std::optional<T> result() const { return coro_.promise().result; }

  auto operator co_await() {
    struct awaiter {
      Task &t;

      bool await_ready() const noexcept { return false; };
      void await_suspend(std::coroutine_handle<> suspended) noexcept {
        t.coro_.promise().parent = suspended;
        t.coro_.resume();
      }
      std::optional<T> await_resume() const noexcept {
        return t.coro_.promise().result;
      }
    };

    return awaiter{*this};
  }

private:
  std::coroutine_handle<promise_type> coro_;
};

template <typename T> struct Task<T>::promise_type {
  std::optional<T> result;
  std::coroutine_handle<> parent{};

  Task get_return_object() {
    return {std::coroutine_handle<promise_type>::from_promise(*this)};
  }

  auto unhandled_exception() { std::terminate(); }

  void return_value(T &&value) { result.emplace(std::move(value)); }

  std::suspend_always initial_suspend() noexcept { return {}; }
  auto final_suspend() noexcept {
    struct final_awaiter {
      bool await_ready() const noexcept { return false; }

      std::coroutine_handle<>
      await_suspend(std::coroutine_handle<promise_type> suspended) noexcept {
        if (suspended.promise().parent) {
          return suspended.promise().parent;
        } else {
          return std::noop_coroutine();
        }
      }

      void await_resume() const noexcept {}
    };

    return final_awaiter{};
  }
};

Task<int> foo(Timer &timer) {
  std::println("Before suspend");
  co_await timer;
  std::println("After suspend");
  co_return 42;
}

Task<int> chained(Timer &timer) {
  std::println("Outer coroutine start");
  auto x = co_await foo(timer);
  std::println("Outer coroutine end: {}", x.value_or(-1));
  co_return 69;
}

int main() {
  auto timer = Timer{};
  auto task_handle = chained(timer);

  task_handle.start();

  std::this_thread::sleep_for(std::chrono::seconds(1));
  timer.tick();
  timer.tick(); // should do nothing

  std::println("Result: {}", task_handle.result().value_or(-1));
}
