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