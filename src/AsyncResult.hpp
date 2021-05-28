#pragma once

enum AsyncState { Pending = 0, Resolved };

template <typename T> class AsyncResult {
public:
  static AsyncResult<T> pending() { return AsyncResult<T>(); }

  static AsyncResult<T> resolve(T input) { return AsyncResult<T>(input); }

  AsyncResult() { this->state = AsyncState::Pending; }
  AsyncResult(T input) {
    this->value = input;
    this->state = AsyncState::Resolved;
  }

  bool is_pending() { return this->state == AsyncState::Pending; }

  bool is_resolved() { return this->state == AsyncState::Resolved; }

  T *get_value() { return this->is_resolved() ? &value : nullptr; }

private:
  AsyncState state;
  T value;
};

template <> class AsyncResult<void> {
public:
  static AsyncResult<void> pending() {
    return AsyncResult<void>(AsyncState::Pending);
  }

  static AsyncResult<void> resolve() {
    return AsyncResult<void>(AsyncState::Resolved);
  }

  AsyncResult(AsyncState state) { this->state = state; }

  bool is_pending() { return this->state == AsyncState::Pending; }

  bool is_resolved() { return this->state == AsyncState::Resolved; }

private:
  AsyncState state;
};