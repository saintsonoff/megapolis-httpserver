/// @file    parallelhandler.hpp
/// @brief   Thread-pool-based handler decorator for offloading request processing.
/// @author  saintson (pan.aleksandr.off@gmail.com)
/// @date    17.02.2026
/// @copyright Copyright (c) 2026 saintson. All rights reserved.
///            Licensed under the GNU General Public License v3.0 (GPLv3).

#pragma once

// stl
#include <concepts>
#include <memory>
#include <optional>

// boost
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/log/trivial.hpp>

// self
#include <handler.hpp>
#include <logging.hpp>


namespace httpserver {


namespace asio = boost::asio;
namespace beast = boost::beast;


/// @brief Decorator that dispatches handler execution onto an asio::thread_pool.
/// @note  The decorated handler's coroutine is spawned on the pool executor
///        and the result is awaited back on the caller's strand.
class ParallelHandler : public IHandler {
public:
    /// @brief Constructs a ParallelHandler wrapping a concrete handler.
    /// @param handler  The inner handler to execute on the pool.
    /// @param tpool    Reference to the thread pool used for dispatch.
    ParallelHandler(Handler handler, asio::thread_pool& tpool);

public:
    /// @brief Dispatches the request to the thread pool and awaits the result.
    /// @param req  The request context forwarded to the inner handler.
    /// @return An awaitable yielding the inner handler's response.
    asio::awaitable<beast::http::message_generator> handle(RequestContext&& req) override;

private:
    Handler m_handler;           ///< The inner handler to delegate to.
    asio::thread_pool& m_tpool;  ///< Thread pool for concurrent execution.
};


/// @brief Factory that wraps another factory's handler creation in a ParallelHandler.
/// @note  Owns an asio::thread_pool whose size is determined at construction time.
class ParallelHandlerFabric : public IHandlerFabric {
public:

    /// @brief Constructs the parallel fabric with a wrapped inner fabric.
    /// @tparam TPoolArgs  Constructor argument types forwarded to asio::thread_pool.
    /// @param fabric       The inner handler factory to wrap.
    /// @param tpool_args   Arguments forwarded to the asio::thread_pool constructor.
    template<std::convertible_to<asio::thread_pool>... TPoolArgs>
    ParallelHandlerFabric(HandlerFabric fabric, TPoolArgs&&... tpool_args)
            : m_fabric(std::move(fabric)), m_tpool{std::forward<TPoolArgs>(tpool_args)...} {
    }

public:
    /// @brief Creates a ParallelHandler wrapping the inner factory's handler.
    /// @return A shared pointer to the new ParallelHandler.
    Handler create() const override;

    /// @brief Returns the endpoint of the wrapped factory.
    /// @return A non-owning view of the endpoint string.
    std::string_view endpoint() const override;

    /// @brief Returns the HTTP verb filter of the wrapped factory.
    /// @return The required verb, or std::nullopt.
    std::optional<beast::http::verb> method() const override;

private:
    HandlerFabric m_fabric;              ///< The inner handler factory.
    mutable asio::thread_pool m_tpool;   ///< Pool for parallel handler execution.
};


} // namespace httpserver