#pragma once

#include <AsyncResult.hpp>
#include <functional>

template <typename I, typename O> class Future {
public:
  Future(std::function<AsyncResult<O>(I)> callback) {
    this->poll_callback = callback;
  }

  AsyncResult<O> poll(AsyncResult<I> input) {
    if (!input.is_resolved()) {
      return AsyncResult<O>::pending();
    }

    this->result = this->poll_callback(*input.get_value());

    return this->result;
  }

  AsyncResult<O> poll(I input) {
    this->result = this->poll_callback(input);
    return this->result;
  }

  template <typename N> Future<I, N> and_then(Future<O, N> next_future) {
    auto this_future = *this;

    return Future<I, N>(
        [this_future, next_future](I input) mutable -> AsyncResult<N> {
          if (this_future.result.is_pending()) {
            this_future.poll(input);
          }

          return next_future.poll(this_future.result);
        });
  }

  template <typename N> Future<I, N> and_then(std::function<N(O)> fn) {
    return this->and_then<N>(Future<O, N>(
        [fn](O input) { return AsyncResult<N>::resolve(fn(input)); }));
  }

  AsyncResult<O> result = AsyncResult<O>::pending();

private:
  std::function<AsyncResult<O>(I)> poll_callback;
};

template <typename I> class Future<I, void> {
public:
  Future(std::function<AsyncResult<void>(I)> callback) {
    this->poll_callback = callback;
  }

  AsyncResult<void> poll(AsyncResult<I> input) {
    if (!input.is_resolved()) {
      return AsyncResult<void>::pending();
    }

    this->result = this->poll_callback(*input.get_value());

    return this->result;
  }

  AsyncResult<void> poll(I input) {
    this->result = this->poll_callback(input);
    return this->result;
  }

  template <typename N> Future<I, N> and_then(Future<void, N> next_future) {
    auto this_future = *this;

    return Future<I, N>(
        [this_future, next_future](I input) mutable -> AsyncResult<N> {
          if (this_future.result.is_pending()) {
            this_future.poll(input);
          }

          return next_future.poll(this_future.result);
        });
  }

  template <typename N> Future<I, N> and_then(std::function<N(void)> fn) {
    return this->and_then<N>(
        Future<void, N>([fn]() { return AsyncResult<N>::resolve(fn()); }));
  }

  AsyncResult<void> result = AsyncResult<void>::pending();

private:
  std::function<AsyncResult<void>(I)> poll_callback;
};

template <typename O> class Future<void, O> {
public:
  Future(std::function<AsyncResult<O>()> callback) {
    this->poll_callback = callback;
  }

  AsyncResult<O> poll(AsyncResult<void> input) {
    if (!input.is_resolved()) {
      return AsyncResult<O>::pending();
    }

    this->result = this->poll_callback();

    return this->result;
  }

  AsyncResult<O> poll() {
    this->result = this->poll_callback();
    return this->result;
  }

  template <typename N> Future<void, N> and_then(Future<O, N> next_future) {
    auto this_future = *this;

    return Future<void, N>(
        [this_future, next_future]() mutable -> AsyncResult<N> {
          if (this_future.result.is_pending()) {
            this_future.poll();
          }

          return next_future.poll(this_future.result);
        });
  }

  template <typename N> Future<void, N> and_then(std::function<void(O)> fn) {
    return this->and_then<N>(Future<O, N>([fn](O input) {
      fn(input);
      return AsyncResult<N>::resolve();
    }));
  }

  AsyncResult<O> result = AsyncResult<O>::pending();

private:
  std::function<AsyncResult<O>()> poll_callback;
};

template <> class Future<void, void> {
public:
  Future(std::function<AsyncResult<void>()> callback) {
    this->poll_callback = callback;
  }

  AsyncResult<void> poll(AsyncResult<void> input) {
    if (!input.is_resolved()) {
      return AsyncResult<void>::pending();
    }

    this->result = this->poll_callback();

    return this->result;
  }

  AsyncResult<void> poll() {
    this->result = this->poll_callback();
    return this->result;
  }

  template <typename N> Future<void, N> and_then(Future<void, N> next_future) {
    auto this_future = *this;

    return Future<void, N>(
        [this_future, next_future]() mutable -> AsyncResult<N> {
          if (this_future.result.is_pending()) {
            this_future.poll();
          }

          return next_future.poll(this_future.result);
        });
  }

  template <typename N> Future<void, N> and_then(std::function<void()> fn) {
    return this->and_then<N>(Future<void, N>([fn]() {
      fn();
      return AsyncResult<N>::resolve();
    }));
  }

  AsyncResult<void> result = AsyncResult<void>::pending();

private:
  std::function<AsyncResult<void>()> poll_callback;
};