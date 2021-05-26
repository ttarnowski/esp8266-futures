#include <gtest/gtest.h>
#include <iostream>

template <typename T, typename U> class Future {
public:
  Future(std::function<U(T)> fn) { this->fn = fn; }

  template <typename N> Future<T, N> and_then(Future<U, N> f2) {
    auto fn = this->fn;
    return Future<T, N>([fn, f2](T input) -> N { return f2.fn(fn(input)); });
  }
  template <typename N> Future<T, N> and_then(std::function<N(U)> fn2) {
    auto fn = this->fn;
    return Future<T, N>([fn, fn2](T input) -> N { return fn2(fn(input)); });
  }

  std::function<U(T)> fn;
};

TEST(test, testing) {
  Future<int, int> f1([](int val) { return val * 3; });
  Future<int, bool> f2([](int val) { return val % 2 == 0; });

  auto f = f1.and_then<bool>(f2).and_then<const char *>(
      [](bool v) { return v ? "true" : "false"; });

  ASSERT_EQ(f.fn(2), "true");
  ASSERT_EQ(f.fn(3), "false");
}
