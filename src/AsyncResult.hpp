#pragma once

#include <iostream>
#include <memory>

enum AsyncState { Pending = 0, Resolved, Rejected };

class ErrorBase {
public:
  virtual ~ErrorBase() = 0;
  virtual operator const char *() = 0;
};

ErrorBase::~ErrorBase() {}

class Error : public ErrorBase {
public:
  Error() { this->err = "error"; }

  Error(ErrorBase *err) { this->err = (const char *)err; }

  Error(const char *err) { this->err = err; }

  operator const char *() override { return this->err; }

  const char *err;
};

template <typename T, typename E = Error> class AsyncResult {
public:
  static AsyncResult<T, E> pending() { return AsyncResult<T, E>(); }

  static AsyncResult<T, E> reject(E err) { return AsyncResult<T, E>(err); }

  static AsyncResult<T, E> resolve(T input) { return AsyncResult<T, E>(input); }

  AsyncResult() { this->state = AsyncState::Pending; }
  AsyncResult(T input) {
    this->value = input;
    this->state = AsyncState::Resolved;
  }

  AsyncResult(E e) {
    this->err = e;
    this->state = AsyncState::Rejected;
  }

  bool is_pending() { return this->state == AsyncState::Pending; }

  bool is_resolved() { return this->state == AsyncState::Resolved; }

  bool is_rejected() { return this->state == AsyncState::Rejected; }

  T *get_value() { return this->is_resolved() ? &value : nullptr; }
  E *get_error() { return this->is_rejected() ? &err : nullptr; }

private:
  AsyncState state;
  T value;
  E err;
};