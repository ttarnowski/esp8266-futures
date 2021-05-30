#pragma once

#include <AsyncResult.hpp>
#include <functional>

template <typename I, typename O, typename E = Error> class Future {
public:
  Future(std::function<AsyncResult<O, E>(I)> callback) {
    this->poll_callback = callback;
  }

  AsyncResult<O, E> poll(I input) {
    this->result = this->poll_callback(input);
    return this->result;
  }

  template <typename N, typename G = Error>
  Future<I, N, G> and_then(Future<O, N, G> next_future) {
    auto this_future = *this;

    return Future<I, N, G>([this_future,
                            next_future](I input) mutable -> AsyncResult<N, G> {
      if (this_future.result.is_pending()) {
        this_future.poll(input);
      }

      if (this_future.result.is_pending()) {
        return AsyncResult<N, G>::pending();
      }

      if (this_future.result.is_rejected()) {
        return AsyncResult<N, G>::reject(G(*this_future.result.get_error()));
      }

      return next_future.poll(*this_future.result.get_value());
    });
  }

  template <typename N> Future<I, N> and_then(std::function<N(O)> fn) {
    return this->and_then<N>(Future<O, N>(
        [fn](O input) { return AsyncResult<N>::resolve(fn(input)); }));
  }

  AsyncResult<O, E> result = AsyncResult<O, E>::pending();

private:
  std::function<AsyncResult<O, E>(I)> poll_callback;
};