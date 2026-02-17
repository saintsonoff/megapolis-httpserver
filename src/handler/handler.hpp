/// @file    handler.hpp
/// @brief   Core handler abstractions and request context for the HTTP server.
/// @author  saintson (pan.aleksandr.off@gmail.com)
/// @date    17.02.2026
/// @copyright Copyright (c) 2026 saintson. All rights reserved.
///            Licensed under the GNU General Public License v3.0 (GPLv3).

#pragma once

// stl
#include <string>
#include <string_view>
#include <memory>
#include <ranges>
#include <concepts>
#include <utility>
#include <optional>

// boost
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>

// self
#include <logging.hpp>


/// @brief Root namespace for the megapolis HTTP server library.
namespace httpserver {


namespace beast = boost::beast;
namespace asio = boost::asio;


/// @brief Bundles the incremental-read state passed from the session to a handler.
/// @note  The pointers remain valid for the lifetime of the enclosing session coroutine.
///        Handlers consume the body through the parser; the socket and buffer are
///        required for async_read_some calls inside the handler.
struct RequestContext {
    beast::http::request_parser<beast::http::buffer_body>* parser;  ///< Incremental HTTP request parser (headers already read).
    beast::tcp_stream* socket;                                       ///< TCP stream for the current connection.
    beast::flat_buffer* buffer;                                      ///< Read buffer shared with the session.
};

class IHandler;
/// @brief Shared-ownership alias for polymorphic request handlers.
using Handler = std::shared_ptr<IHandler>;

class IHandlerFabric;
/// @brief Shared-ownership alias for polymorphic handler factories.
using HandlerFabric = std::shared_ptr<IHandlerFabric>;


/// @brief Constrains a range whose value type is HandlerFabric.
/// @tparam HandlerFabricsRangeType A std::ranges::range of HandlerFabric elements.
template<typename HandlerFabricsRangeType>
concept IsHandlerFabricsRange = std::ranges::range<HandlerFabricsRangeType>
    && std::same_as<std::ranges::range_value_t<HandlerFabricsRangeType>, HandlerFabric>;


/// @brief Abstract interface for HTTP request handlers.
/// @note  Implementations are expected to fully consume the request body
///        (via incremental async_read_some) before returning.
class IHandler {
public:
    /// @brief Processes an HTTP request and produces a response.
    /// @param req  The request context containing parser, socket, and buffer.
    /// @return An awaitable that yields the HTTP response as a message_generator.
    virtual asio::awaitable<beast::http::message_generator> handle(RequestContext&& req) = 0;

    /// @brief Virtual destructor.
    virtual ~IHandler() {};
};

/// @brief Logs connection details (peer address, method, target, keep-alive) at debug level.
/// @param req The request context whose parser headers are inspected.
inline void connection_logging(const RequestContext& req) {
    auto& msg = req.parser->get();
    auto ep = req.socket->socket().remote_endpoint();
    auto peer = std::format("{}:{}", ep.address().to_string(), ep.port());
    BOOST_LOG_TRIVIAL(debug) << std::format("[{}] {} {} keep_alive={}", 
                                        peer, std::string{msg.method_string()}, 
                                        std::string{msg.target()}, msg.keep_alive());
}

/// @brief Abstract factory interface for creating IHandler instances.
class IHandlerFabric {
public:
    /// @brief Creates a new handler instance.
    /// @return A shared pointer to the newly created handler.
    virtual Handler create() const = 0;

    /// @brief Returns the URL endpoint this factory is registered for.
    /// @return A non-owning view of the endpoint string (e.g. "/upload").
    virtual std::string_view endpoint() const = 0;

    /// @brief Returns the optional HTTP verb filter for this factory.
    /// @return The required verb, or std::nullopt if any verb is accepted.
    virtual std::optional<beast::http::verb> method() const = 0;

    /// @brief Virtual destructor.
    virtual ~IHandlerFabric() {};
};


/// @brief Generic handler factory that creates handlers of a concrete type.
/// @tparam HandlerType A concrete class derived from IHandler.
template<std::derived_from<IHandler> HandlerType>
class HandlerFabricImpl : public IHandlerFabric {
public:
    /// @brief Constructs a factory bound to a specific HTTP method and endpoint.
    /// @param method   The HTTP verb to match (e.g. beast::http::verb::post).
    /// @param endpoint The URL path to match (e.g. "/upload").
    HandlerFabricImpl(beast::http::verb method, std::string endpoint);

    /// @brief Constructs a factory bound to an endpoint, accepting any HTTP method.
    /// @param endpoint The URL path to match.
    HandlerFabricImpl(std::string endpoint);

public:
    /// @brief Creates a new HandlerType instance.
    /// @return A shared pointer to the newly created handler.
    Handler create() const override;

    /// @brief Returns the registered endpoint.
    /// @return A non-owning view of the endpoint string.
    std::string_view endpoint() const override;

    /// @brief Returns the optional HTTP verb filter.
    /// @return The required verb, or std::nullopt.
    std::optional<beast::http::verb> method() const override;

private:
    std::optional<beast::http::verb> m_method;  ///< Optional HTTP verb filter.
    std::string m_endpoint;                      ///< The URL path this factory matches.
};


} // namespace httpserver


// header only implementation
#include "handler.tpp"