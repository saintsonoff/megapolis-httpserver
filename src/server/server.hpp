/// @file    server.hpp
/// @brief   Asynchronous HTTP server built on Boost.Beast and Boost.Asio coroutines.
/// @author  saintson (pan.aleksandr.off@gmail.com)
/// @date    17.02.2026
/// @copyright Copyright (c) 2026 saintson. All rights reserved.
///            Licensed under the GNU General Public License v3.0 (GPLv3).

#pragma once

// stl
#include <string>
#include <expected>
#include <chrono>
#include <limits>

// boost
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/log/trivial.hpp>

// self
#include <logging.hpp>
#include <handler.hpp>
#include <router.hpp>


namespace httpserver {


namespace beast = boost::beast;
namespace asio = boost::asio;


/// @brief Coroutine-based HTTP server that accepts connections and dispatches
///        requests through a compile-time-typed router.
/// @tparam RouterType A type satisfying the IsRouter concept.
/// @note  Runs a single-threaded io_context.  Graceful shutdown is triggered by
///        SIGINT or SIGTERM.
template<IsRouter RouterType>
class Server {
public:
    /// @brief Constructs the server and starts listening on the given port.
    /// @param port     TCP port to bind (e.g. 1616).
    /// @param handlers The router instance that maps URLs to handler factories.
    Server(beast::net::ip::port_type port, RouterType handlers);

    /// @brief Stops the io_context and logs shutdown.
    ~Server();

public:
    /// @brief Enters the io_context event loop (blocks until stopped).
    /// @return An empty expected on success, or an error string on exception.
    std::expected<void, std::string> run();

private:
    /// @brief Coroutine that accepts new TCP connections in a loop.
    /// @param port The port to listen on.
    /// @return An awaitable<void>.
    asio::awaitable<void> listen(beast::net::ip::port_type port);

    /// @brief Coroutine that handles a single HTTP session (read-header, route, respond).
    /// @param stream The TCP stream for the accepted connection.
    /// @return An awaitable<void>.
    asio::awaitable<void> session(beast::tcp_stream stream);

private:
    /// @brief Logs a Boost error_code at the appropriate severity level.
    /// @param error_code The error code to inspect.
    /// @param peer       A "host:port" string identifying the remote peer.
    /// @return true if an error was present (caller should break), false otherwise.
    static bool error_logging(const boost::system::error_code& error_code, const std::string& peer);

private:
    RouterType m_router;         ///< The request router.
    asio::io_context m_ctx;      ///< Single-threaded I/O context.
    asio::signal_set m_signals;  ///< Signal handler for graceful shutdown.
};


} // namespace httpserver



// header only implemetation
#include "server.tpp"