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

template <typename I, typename E> class Future<I, void, E> {
public:
  Future(std::function<AsyncResult<void, E>(I)> callback) {
    this->poll_callback = callback;
  }

  AsyncResult<void, E> poll(I input) {
    this->result = this->poll_callback(input);
    return this->result;
  }

  template <typename N, typename G = Error>
  Future<I, N, G> and_then(Future<void, N, G> next_future) {
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

      return next_future.poll();
    });
  }

  template <typename N> Future<I, N> and_then(std::function<N(void)> fn) {
    return this->and_then<N>(
        Future<void, N>([fn]() { return AsyncResult<N>::resolve(fn()); }));
  }

  AsyncResult<void, E> result = AsyncResult<void, E>::pending();

private:
  std::function<AsyncResult<void, E>(I)> poll_callback;
};

template <typename O, typename E> class Future<void, O, E> {
public:
  Future(std::function<AsyncResult<O, E>()> callback) {
    this->poll_callback = callback;
  }

  AsyncResult<O, E> poll() {
    this->result = this->poll_callback();
    return this->result;
  }

  template <typename N, typename G = Error>
  Future<void, N, G> and_then(Future<O, N, G> next_future) {
    auto this_future = *this;

    return Future<void, N, G>([this_future,
                               next_future]() mutable -> AsyncResult<N, G> {
      if (this_future.result.is_pending()) {
        this_future.poll();
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

  template <typename N, typename std::enable_if<
                            !std::is_same<N, void>::value>::type * = nullptr>
  Future<void, N> and_then(std::function<N(O)> fn) {
    return this->and_then<N>(Future<O, N>(
        [fn](O input) { return AsyncResult<N>::resolve(fn(input)); }));
  }

  template <typename N, typename std::enable_if<
                            std::is_same<N, void>::value>::type * = nullptr>
  Future<void, N> and_then(std::function<N(O)> fn) {
    return this->and_then<N>(Future<O, N>([fn](O input) {
      fn(input);
      return AsyncResult<N>::resolve();
    }));
  }

  AsyncResult<O, E> result = AsyncResult<O, E>::pending();

private:
  std::function<AsyncResult<O, E>()> poll_callback;
};

template <typename E> class Future<void, void, E> {
public:
  Future(std::function<AsyncResult<void, E>()> callback) {
    this->poll_callback = callback;
  }

  AsyncResult<void, E> poll() {
    this->result = this->poll_callback();
    return this->result;
  }

  template <typename N, typename G = Error>
  Future<void, N, G> and_then(Future<void, N, G> next_future) {
    auto this_future = *this;

    return Future<void, N, G>([this_future,
                               next_future]() mutable -> AsyncResult<N, G> {
      if (this_future.result.is_pending()) {
        this_future.poll();
      }

      if (this_future.result.is_pending()) {
        return AsyncResult<N, G>::pending();
      }

      if (this_future.result.is_rejected()) {
        return AsyncResult<N, G>::reject(G(*this_future.result.get_error()));
      }

      return next_future.poll();
    });
  }

  template <typename N, typename std::enable_if<
                            !std::is_same<N, void>::value>::type * = nullptr>
  Future<void, N> and_then(std::function<N()> fn) {
    return this->and_then<N>(
        Future<void, N>([fn]() { return AsyncResult<N>::resolve(fn()); }));
  }

  template <typename N, typename std::enable_if<
                            std::is_same<N, void>::value>::type * = nullptr>
  Future<void, N> and_then(std::function<N()> fn) {
    return this->and_then<N>(Future<void, N>([fn]() {
      fn();
      return AsyncResult<N>::resolve();
    }));
  }

  AsyncResult<void, E> result = AsyncResult<void, E>::pending();

private:
  std::function<AsyncResult<void, E>()> poll_callback;
};
