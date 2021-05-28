#include <Future.hpp>
#include <gtest/gtest.h>
#include <iostream>

unsigned long millis() {
  return std::chrono::system_clock::now().time_since_epoch() /
         std::chrono::milliseconds(1);
}

TEST(Future, test_result_has_default_state_pending_for_all_types_of_futures) {
  Future<int, int> f1([](int) { return AsyncResult<int>::pending(); });
  Future<void, int> f2([]() { return AsyncResult<int>::pending(); });
  Future<int, void> f3([](int) { return AsyncResult<void>::pending(); });
  Future<void, void> f4([]() { return AsyncResult<void>::pending(); });

  ASSERT_TRUE(f1.result.is_pending());
  ASSERT_TRUE(f2.result.is_pending());
  ASSERT_TRUE(f3.result.is_pending());
  ASSERT_TRUE(f4.result.is_pending());
}

TEST(
    Future,
    test_future_changes_its_result_state_to_resolved_if_poll_fn_returns_resolved_result) {
  int expected = 1;

  Future<int, int> f1([](int val) { return AsyncResult<int>::resolve(val); });
  Future<void, int> f2(
      [expected]() { return AsyncResult<int>::resolve(expected); });
  Future<int, void> f3([](int) { return AsyncResult<void>::resolve(); });
  Future<void, void> f4([]() { return AsyncResult<void>::resolve(); });

  ASSERT_TRUE(f1.poll(expected).is_resolved());
  ASSERT_TRUE(f2.poll().is_resolved());
  ASSERT_TRUE(f3.poll(1).is_resolved());
  ASSERT_TRUE(f4.poll().is_resolved());

  ASSERT_TRUE(f1.result.is_resolved());
  ASSERT_TRUE(f2.result.is_resolved());
  ASSERT_TRUE(f3.result.is_resolved());
  ASSERT_TRUE(f4.result.is_resolved());

  ASSERT_EQ(*f1.result.get_value(), expected);
  ASSERT_EQ(*f2.result.get_value(), expected);
}

TEST(Future,
     test_futures_chain_is_pending_when_first_future_in_chain_is_pending) {

  Future<int, int> f1([](int) { return AsyncResult<int>::pending(); });
  Future<void, int> f2([]() { return AsyncResult<int>::pending(); });
  Future<int, void> f3([](int) { return AsyncResult<void>::pending(); });
  Future<void, void> f4([]() { return AsyncResult<void>::pending(); });

  auto c1 = f1.and_then(
      Future<int, void>([](int) { return AsyncResult<void>::resolve(); }));
  auto c2 = f2.and_then(
      Future<int, void>([](int) { return AsyncResult<void>::resolve(); }));
  auto c3 = f3.and_then(
      Future<void, void>([]() { return AsyncResult<void>::resolve(); }));
  auto c4 = f4.and_then(
      Future<void, void>([]() { return AsyncResult<void>::resolve(); }));

  ASSERT_TRUE(c1.poll(2).is_pending());
  ASSERT_TRUE(c1.result.is_pending());
  ASSERT_TRUE(c2.poll().is_pending());
  ASSERT_TRUE(c2.result.is_pending());
  ASSERT_TRUE(c3.poll(2).is_pending());
  ASSERT_TRUE(c3.result.is_pending());
  ASSERT_TRUE(c4.poll().is_pending());
  ASSERT_TRUE(c4.result.is_pending());
}

TEST(
    Future,
    test_futures_chain_resolves_with_last_future_return_value_if_all_previous_futures_in_chain_have_resolved) {
  int input = 3;
  int expected = input * 2;

  Future<int, int> f1([](int val) { return AsyncResult<int>::resolve(val); });
  Future<void, int> f2(
      [expected]() { return AsyncResult<int>::resolve(expected); });
  Future<int, void> f3([](int val) { return AsyncResult<void>::resolve(); });
  Future<void, void> f4([]() { return AsyncResult<void>::resolve(); });

  auto c1 = f1.and_then(Future<int, int>(
      [](int val) { return AsyncResult<int>::resolve(val * 2); }));
  auto c2 = f2.and_then(Future<int, int>(
      [expected](int) { return AsyncResult<int>::resolve(expected); }));
  auto c3 = f3.and_then(Future<void, int>(
      [expected]() { return AsyncResult<int>::resolve(expected); }));
  auto c4 = f4.and_then(Future<void, int>(
      [expected]() { return AsyncResult<int>::resolve(expected); }));

  ASSERT_TRUE(c1.poll(input).is_resolved());
  ASSERT_EQ(*c1.result.get_value(), expected);
  ASSERT_TRUE(c2.poll().is_resolved());
  ASSERT_EQ(*c2.result.get_value(), expected);
  ASSERT_TRUE(c3.poll(input).is_resolved());
  ASSERT_EQ(*c3.result.get_value(), expected);
  ASSERT_TRUE(c4.poll().is_resolved());
  ASSERT_EQ(*c4.result.get_value(), expected);
}

TEST(
    Future,
    test_long_futures_chain_resolves_with_expected_value_after_expected_amount_of_waiting_time) {
  unsigned long time = millis() + 50;

  Future<int, int> f1([time](int val) {
    if (millis() >= time) {
      return AsyncResult<int>::resolve(val * 3);
    }

    return AsyncResult<int>::pending();
  });

  Future<int, bool> f2(
      [](int val) { return AsyncResult<bool>::resolve(val % 2 == 0); });

  auto f = f1.and_then<bool>(f2).and_then<const char *>(
      [](bool val) { return val ? "true" : "false"; });

  auto g = f;

  while (f.poll(2).is_pending() || g.poll(3).is_pending())
    ;

  ASSERT_EQ(*f.result.get_value(), "true");
  ASSERT_EQ(*g.result.get_value(), "false");
  ASSERT_GE(millis(), time);
}
