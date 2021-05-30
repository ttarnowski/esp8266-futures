#include <Future.hpp>
#include <gtest/gtest.h>
#include <iostream>

unsigned long millis() {
  return std::chrono::system_clock::now().time_since_epoch() /
         std::chrono::milliseconds(1);
}

class MyError : public ErrorBase {
public:
  MyError() {}

  MyError(ErrorBase *err) {
    sprintf(this->msg, "myError: %s", (const char *)err);
  }

  MyError(const char *err) { sprintf(this->msg, "myError: %s", err); }

  operator const char *() override { return this->msg; }

  char msg[64];
};

TEST(Future, testing) {
  unsigned long time = millis() + 300;

  Future<int, int> f1([time](int val) {
    // return AsyncResult<int>::resolve(MyError("timeout failed"));
    if (millis() >= time) {
      return AsyncResult<int>::resolve(val * 3);
    }

    return AsyncResult<int>::pending();
  });

  Future<int, bool, MyError> f2([](int val) {
    return AsyncResult<bool, MyError>::reject(MyError("timeout failed"));
    // return AsyncResult<bool>::resolve(val % 2 == 0);
  });

  auto f = f1.and_then<bool, MyError>(f2).and_then<const char *>(
      [](bool val) { return val ? "true" : "false"; });

  auto g = f;

  while (f.poll(2).is_pending() || g.poll(3).is_pending())
    ;

  ASSERT_TRUE(f.result.is_rejected());
  ASSERT_STREQ(*f.result.get_error(), "myError: timeout failed");
}
